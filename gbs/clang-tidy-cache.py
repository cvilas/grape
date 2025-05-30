#!/usr/bin/env python3
# coding: UTF-8
# Copyright (c) 2019-2025 Matus Chochlik
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
#  http://www.boost.org/LICENSE_1_0.txt
"""
clang-tidy-cache is a command-line application which "wraps" invocations
of the `clang-tidy` static analysis tool and caches the results of successful
runs of `clang-tidy`. On subsequent invocations of `clang-tidy` on an unchanged
translation unit, the result is retrieved from the cache and `clang-tidy`
is not executed. For most C/C++ projects this allows to have static analysis
checks enabled without paying the cost of excessive build times when re-checking
the same unchanged source code.
"""

import errno
import getpass
import hashlib
import json
import logging
import os
import re
import shlex
import shutil
import subprocess
import sys
import tempfile
import time
import traceback
from typing import List, Optional

try:
    import redis
except ImportError:
    redis = None

# ------------------------------------------------------------------------------
def getenv_boolean_flag(name: str) -> bool:
    "Returns a boolean value of an environment variable."
    return os.getenv(name, "0").lower() in ["true", "1", "yes", "y", "on"]

# ------------------------------------------------------------------------------
def mkdir_p(path: os.PathLike) -> None:
    "Ensures that the directory tree with the specified path exists."
    try:
        os.makedirs(path)
    except OSError as os_error:
        if os_error.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

# ------------------------------------------------------------------------------
class ClangTidyCacheOpts:
    "Class holding the parsed command-line options."
    # --------------------------------------------------------------------------
    def __init__(self, log, args: List[str]):
        self._log = log

        if len(args) < 1:
            self._log.error("Missing arguments")

        self._original_args = args
        self._clang_tidy_args = []
        self._compiler_args = []
        self._cache_dir = None
        self._compile_commands_db = None

        self._strip_list = os.getenv("CTCACHE_STRIP", "").split(os.pathsep)

        args = self._split_compiler_clang_tidy_args(args)
        self._adjust_compiler_args()

    # --------------------------------------------------------------------------
    def __repr__(self) -> str:
        return f"ClangTidyCacheOpts(" \
                    f"clang_tidy_args:{self._clang_tidy_args}," \
                    f"compiler_args:{self._compiler_args}," \
                    f"original_args:{self._original_args}" \
                f")"

    # --------------------------------------------------------------------------
    def running_on_msvc(self) -> bool:
        "Indicates if MSVC is the used compiler."
        if self._compiler_args:
            return os.path.basename(self._compiler_args[0]) == "cl.exe"
        return False

    # --------------------------------------------------------------------------
    def running_on_clang_cl(self) -> bool:
        "Indicates if clang-cl is the used compiler."
        if self._compiler_args:
            return os.path.basename(self._compiler_args[0]) == "clang-cl.exe"
        return False

    # --------------------------------------------------------------------------
    def _split_compiler_clang_tidy_args(self, args: List[str]) -> List[str]:
        # splits arguments starting with - on the first =
        args = [arg.split('=', 1) if arg.startswith('-p') else [arg] for arg in args]
        args = [arg for sub in args for arg in sub]

        if args.count("--") == 1:
            # Invoked with compiler args on the actual command line
            i = args.index("--")
            self._clang_tidy_args = args[:i]
            self._compiler_args = args[i+1:]
        elif args.count("-p") == 1:
            # Invoked with compiler args in a compile commands json db
            i = args.index("-p")
            self._clang_tidy_args = args

            i += 1
            if i >= len(args):
                return None

            cdb_path = args[i]
            if os.path.isdir(cdb_path):
                cdb_path = os.path.join(cdb_path, "compile_commands.json")
            self._load_compile_command_db(cdb_path)

            i += 1
            if i >= len(args):
                return None

            # This assumes that the filename occurs after the -p <cdb path>
            # and that there is only one of them
            filenames = [arg for arg in args[i:] if not arg.startswith("-")]
            if len(filenames) > 0:
                self._compiler_args = self._compiler_args_for(filenames[0])
        else:
            # Invoked as pure clang-tidy command
            self._clang_tidy_args = args[1:]
        return args

    # --------------------------------------------------------------------------
    def _adjust_compiler_args(self) -> None:
        if self._compiler_args:
            pos = next((pos for pos, arg in enumerate(self._compiler_args) if arg.startswith('-D')), 1)
            self._compiler_args.insert(pos, "-D__clang_analyzer__=1")
            for i in range(1, len(self._compiler_args)):
                if self._compiler_args[i-1] in ["-o", "--output"]:
                    self._compiler_args[i] = "-"
                if self._compiler_args[i-1] in ["-c"]:
                    self._compiler_args[i-1] = "-E"
            for i in range(1, len(self._compiler_args)):
                if self._compiler_args[i-1] in ["-E"]:
                    if self.running_on_msvc():
                        self._compiler_args[i-1] = "-EP"
                    else:
                        self._compiler_args.insert(i, "-P")
                    if self.keep_comments():
                        self._compiler_args.insert(i, "-C")

    # --------------------------------------------------------------------------
    def _load_compile_command_db(self, filename: os.PathLike) -> None:
        try:
            with open(filename) as f:
                cdb = f.read()
                try:
                    js = cdb.replace(r'\\\"', "'").replace("\\", "\\\\")
                    self._compile_commands_db = json.loads(js)
                except json.JSONDecodeError:
                    self._compile_commands_db = json.loads(cdb)

        except Exception as err:
            self._log.error("Loading compile command DB failed: {0}".format(repr(err)))

    # --------------------------------------------------------------------------
    def _compiler_args_for(self, filename: os.PathLike):
        if self._compile_commands_db is None:
            return []

        filename = os.path.expanduser(filename)
        filename = os.path.realpath(filename)

        for command in self._compile_commands_db:
            db_filename = command["file"]
            try:
                if os.path.samefile(filename, db_filename):
                    try:
                        return shlex.split(command["command"])
                    except KeyError:
                        try:
                            return shlex.split(command["arguments"][0])
                        except:
                            return ["clang-tidy"]
            except FileNotFoundError:
                continue

        return []

    # --------------------------------------------------------------------------
    def should_print_dir(self) -> bool:
        try:
            return self._original_args[0] == "--cache-dir"
        except IndexError:
            return False

    # --------------------------------------------------------------------------
    def should_print_stats(self) -> bool:
        try:
            return self._original_args[0] == "--show-stats"
        except IndexError:
            return False

    # --------------------------------------------------------------------------
    def should_print_stats_raw(self) -> bool:
        try:
            return self._original_args[0] == "--print-stats"
        except IndexError:
            return False

    # --------------------------------------------------------------------------
    def should_remove_dir(self) -> bool:
        try:
            return self._original_args[0] == "--clean"
        except IndexError:
            return False

    # --------------------------------------------------------------------------
    def should_zero_stats(self) -> bool:
        try:
            return self._original_args[0] == "--zero-stats"
        except IndexError:
            return False

    # --------------------------------------------------------------------------
    def should_print_usage(self) -> bool:
        return len(self.original_args()) < 1

    # --------------------------------------------------------------------------
    def original_args(self) -> List[str]:
        return self._original_args

    # --------------------------------------------------------------------------
    def clang_tidy_args(self) -> List[str]:
        return self._clang_tidy_args

    # --------------------------------------------------------------------------
    def compiler_args(self) -> List[str]:
        return self._compiler_args

    # --------------------------------------------------------------------------
    @property
    def cache_dir(self) -> os.PathLike:
        if self._cache_dir:
            return self._cache_dir

        try:
            user = getpass.getuser()
        except KeyError:
            user = "unknown"
        self._cache_dir = os.getenv(
            "CTCACHE_DIR",
            os.path.join(
                tempfile.tempdir if tempfile.tempdir else "/tmp", "ctcache-" + user
            ),
        )
        return self._cache_dir

     # --------------------------------------------------------------------------
    def strip_paths(self, text: str) -> str:
        for item in self._strip_list:
            text = re.sub(item, '', text)
        return text

    # --------------------------------------------------------------------------
    def adjust_chunk(self, x: str) -> bytes:
        x = x.strip()
        r = str().encode("utf8")
        if not x.startswith("# "):
            for w in x.split():
                w = w.strip('"')
                if os.path.exists(w):
                    w = os.path.realpath(w)
                w = self.strip_paths(w)
                w.strip()
                if w:
                    r += w.encode("utf8")
        return r

    # --------------------------------------------------------------------------
    def has_s3(self) -> bool:
        return "CTCACHE_S3_BUCKET" in os.environ

    # --------------------------------------------------------------------------
    def s3_bucket(self) -> str:
        return os.getenv("CTCACHE_S3_BUCKET")

    # --------------------------------------------------------------------------
    def s3_bucket_folder(self) -> str:
        return os.getenv("CTCACHE_S3_FOLDER", 'clang-tidy-cache')

    # --------------------------------------------------------------------------
    def s3_no_credentials(self):
        return os.getenv("CTCACHE_S3_NO_CREDENTIALS", "")

    # --------------------------------------------------------------------------
    def s3_read_only(self) -> bool:
        return getenv_boolean_flag("CTCACHE_S3_READ_ONLY")

    # --------------------------------------------------------------------------
    def has_gcs(self) -> bool:
        return "CTCACHE_GCS_BUCKET" in os.environ

    # --------------------------------------------------------------------------
    def gcs_bucket(self) -> str:
        return os.getenv("CTCACHE_GCS_BUCKET")

    # --------------------------------------------------------------------------
    def gcs_bucket_folder(self) -> str:
        return os.getenv("CTCACHE_GCS_FOLDER", 'clang-tidy-cache')

    # --------------------------------------------------------------------------
    def gcs_no_credentials(self):
        return os.getenv("CTCACHE_GCS_NO_CREDENTIALS")

    # --------------------------------------------------------------------------
    def gcs_read_only(self) -> bool:
        return getenv_boolean_flag("CTCACHE_GCS_READ_ONLY")

    # --------------------------------------------------------------------------
    def cache_locally(self) -> bool:
        return getenv_boolean_flag("CTCACHE_LOCAL")

    # --------------------------------------------------------------------------
    def no_local_stats(self) -> bool:
        return getenv_boolean_flag("CTCACHE_NO_LOCAL_STATS")

    # --------------------------------------------------------------------------
    def no_local_writeback(self) -> bool:
        return getenv_boolean_flag("CTCACHE_NO_LOCAL_WRITEBACK")

    # --------------------------------------------------------------------------
    def has_host(self) -> bool:
        return "CTCACHE_HOST" in os.environ

    # --------------------------------------------------------------------------
    def rest_host(self) -> str:
        return os.getenv("CTCACHE_HOST", "localhost")

    # --------------------------------------------------------------------------
    def rest_proto(self) -> str:
        return os.getenv("CTCACHE_PROTO", "http")

    # --------------------------------------------------------------------------
    def rest_port(self) -> int:
        return int(os.getenv("CTCACHE_PORT", 5000))

    # --------------------------------------------------------------------------
    def rest_host_read_only(self) -> bool:
        return getenv_boolean_flag("CTCACHE_HOST_READ_ONLY")

    # --------------------------------------------------------------------------
    def save_output(self) -> bool:
        return getenv_boolean_flag("CTCACHE_SAVE_OUTPUT")

    # --------------------------------------------------------------------------
    def ignore_output(self) -> bool:
        return self.save_output() or "CTCACHE_IGNORE_OUTPUT" in os.environ

    # --------------------------------------------------------------------------
    def save_all(self) -> bool:
        return self.save_output() or "CTCACHE_SAVE_ALL" in os.environ

    # --------------------------------------------------------------------------
    def debug_enabled(self) -> bool:
        return getenv_boolean_flag("CTCACHE_DEBUG")

    # --------------------------------------------------------------------------
    def dump_enabled(self) -> bool:
        return getenv_boolean_flag("CTCACHE_DUMP")

    # --------------------------------------------------------------------------
    def dump_dir(self) -> os.PathLike:
        return os.getenv("CTCACHE_DUMP_DIR", tempfile.gettempdir())

    # --------------------------------------------------------------------------
    def strip_src(self) -> bool:
        return getenv_boolean_flag("CTCACHE_STRIP_SRC")

    # --------------------------------------------------------------------------
    def keep_comments(self) -> bool:
        return getenv_boolean_flag("CTCACHE_KEEP_COMMENTS")

    # --------------------------------------------------------------------------
    def exclude_hash_regex(self) -> str:
        return os.getenv("CTCACHE_EXCLUDE_HASH_REGEX")

    # --------------------------------------------------------------------------
    def exclude_hash(self, chunk: bytes) -> bool:
        return self.exclude_hash_regex() is not None and \
            re.match(self.exclude_hash_regex(), chunk.decode("utf8"))

    # --------------------------------------------------------------------------
    def exclude_user_config(self) -> bool:
        return getenv_boolean_flag("CTCACHE_EXCLUDE_USER_CONFIG")

    # --------------------------------------------------------------------------
    def has_redis_host(self) -> bool:
        return "CTCACHE_REDIS_HOST" in os.environ

    # --------------------------------------------------------------------------
    def redis_host(self) -> str:
        return os.getenv("CTCACHE_REDIS_HOST", "")

    # --------------------------------------------------------------------------
    def redis_port(self) -> int:
        return int(os.getenv("CTCACHE_REDIS_PORT", "6379"))

    # --------------------------------------------------------------------------
    def redis_db(self) -> int:
        return int(os.getenv("CTCACHE_REDIS_DB", "0"))

    # --------------------------------------------------------------------------
    def redis_username(self) -> str:
        return os.getenv("CTCACHE_REDIS_USERNAME", "")

    # --------------------------------------------------------------------------
    def redis_password(self) -> str:
        return os.getenv("CTCACHE_REDIS_PASSWORD", "")

    # --------------------------------------------------------------------------
    def redis_connect_timeout(self) -> float:
        return float(os.getenv("CTCACHE_REDIS_CONNECT_TIMEOUT", "0.1"))

    # --------------------------------------------------------------------------
    def redis_socket_timeout(self) -> float:
        return float(os.getenv("CTCACHE_REDIS_OPERATION_TIMEOUT", "10.0"))

    # --------------------------------------------------------------------------
    def redis_cache_ttl(self) -> float:
        ttl = int(os.getenv("CTCACHE_REDIS_CACHE_TTL", "-1"))
        if ttl < 0:
            return None
        return ttl

    # --------------------------------------------------------------------------
    def redis_namespace(self) -> str:
        return os.getenv("CTCACHE_REDIS_NAMESPACE", "ctcache/")

    # --------------------------------------------------------------------------
    def redis_read_only(self) -> bool:
        return getenv_boolean_flag("CTCACHE_REDIS_READ_ONLY")

# ------------------------------------------------------------------------------
class ClangTidyCacheHash:
    # --------------------------------------------------------------------------
    def _opendump(self, opts):
        return open(os.path.join(opts.dump_dir(), "ctcache.dump"), "ab")

    # --------------------------------------------------------------------------
    def __init__(self, opts):
        self._hash = hashlib.sha1()
        if opts.dump_enabled():
            self._dump = self._opendump(opts)
        else:
            self._dump = None
        assert self._dump or not opts.dump_enabled()

    # --------------------------------------------------------------------------
    def __del__(self):
        if self._dump:
            self._dump.close()

    # --------------------------------------------------------------------------
    def update(self, content):
        if content:
            self._hash.update(content)
            if self._dump:
                self._dump.write(content)

    # --------------------------------------------------------------------------
    def hexdigest(self):
        return self._hash.hexdigest()

# ------------------------------------------------------------------------------
class ClangTidyServerCache:
    def __init__(self, log, opts):
        import requests
        self._requests = requests
        self._log = log
        self._opts = opts

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        try:
            query = self._requests.get(self._make_query_url(digest), timeout=3)
            if query.status_code == 200:
                if query.json() is True:
                    return True
                elif query.json() is False:
                    return False
                else:
                    self._log.error("is_cached: Can't connect to server {0}, error {1}".format(
                        self._opts.rest_host(), query.status_code))
        except:
            pass

        return False

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        try:
            query = self._requests.get(self._make_data_url(digest), timeout=3)
            if query.status_code == 200:
                return query.text.encode('UTF-8')
        except:
            pass

        return None

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        self.store_in_cache_with_data(digest, bytes())

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        if self._opts.rest_host_read_only():
            return
        try:
            query = self._requests.put(self._make_data_url(digest), data={'data': data}, timeout=3)
            if query.status_code != 200:
                self._log.error("store_in_cache: Can't store data in server {0}, error {1}".format(
                    self._opts.rest_host(), query.status_code))
        except:
            pass

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        try:
            query = self._requests.get(self._make_stats_url(), timeout=3)
            if query.status_code == 200:
                return query.json()
            else:
                self._log.error("query_stats: Can't connect to server {0}, error {1}".format(
                    self._opts.rest_host(), query.status_code))
        except:
            pass
        return None

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        # Not implemented
        pass

    # --------------------------------------------------------------------------
    def _make_query_url(self, digest):
        return "%(proto)s://%(host)s:%(port)d/is_cached/%(digest)s" % {
            "proto": self._opts.rest_proto(),
            "host": self._opts.rest_host(),
            "port": self._opts.rest_port(),
            "digest": digest
        }

    # --------------------------------------------------------------------------
    def _make_data_url(self, digest):
        return "%(proto)s://%(host)s:%(port)d/cache/%(digest)s" % {
            "proto": self._opts.rest_proto(),
            "host": self._opts.rest_host(),
            "port": self._opts.rest_port(),
            "digest": digest
        }

    # --------------------------------------------------------------------------
    def _make_stats_url(self):
        return "%(proto)s://%(host)s:%(port)d/stats" % {
            "proto": self._opts.rest_proto(),
            "host": self._opts.rest_host(),
            "port": self._opts.rest_port()
        }

# ------------------------------------------------------------------------------
class MultiprocessLock:
    # --------------------------------------------------------------------------
    def __init__(self, lock_path, timeout=3): # timeout 3 seconds
        self._lock_path = os.path.abspath(os.path.expanduser(lock_path))
        self._timeout = timeout
        self._lock_handle = None

    # --------------------------------------------------------------------------
    def acquire(self):
        start_time = time.time()
        while True:
            try:
                # Attempt to create the lock file exclusively
                self._lock_handle = os.open(self._lock_path, os.O_CREAT | os.O_EXCL)
                return self
            except FileExistsError:
                # File is locked, check if the timeout has been exceeded
                if time.time() - start_time > self._timeout:
                    msg = f"Timeout ({self._timeout} seconds) exceeded while acquiring lock."
                    raise RuntimeError(msg)
                # Wait and try again
                time.sleep(0.1)
            except FileNotFoundError:
                # The path to the lock file doesn't exist, create it and retry
                os.makedirs(os.path.dirname(self._lock_path), exist_ok=True)

    # --------------------------------------------------------------------------
    def release(self):
        if self._lock_handle is not None:
            try:
                os.close(self._lock_handle)
                os.unlink(self._lock_path)  # Remove the lock file upon release
            except OSError:
                pass  # Ignore errors if the file doesn't exist or has already been released
            finally:
                self._lock_handle = None

    # --------------------------------------------------------------------------
    def __enter__(self):
        return self.acquire()

    # --------------------------------------------------------------------------
    def __exit__(self, exc_type, exc_value, traceback):
        self.release()

# ------------------------------------------------------------------------------
class ClangTidyCacheStats:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts, name):
        self._log = log
        self._opts = opts
        self._name = name

    # --------------------------------------------------------------------------
    def stats_file(self, digest):
        return os.path.join(self._opts.cache_dir, digest[:2], self._name)

    # --------------------------------------------------------------------------
    def read(self):
        hits, misses = 0, 0
        for i in range(0, 256):
            digest = f'{i:x}'
            file = self.stats_file(digest)
            if os.path.isfile(file):
                h, m = self._read(file)
                hits += h
                misses += m
        return hits, misses

    # --------------------------------------------------------------------------
    def _read(self, file):
        with MultiprocessLock(file + ".lock") as _:
            if os.path.isfile(file):
                with open(file, 'r') as f:
                    return self.read_from_file(f)
            return 0,0

    # --------------------------------------------------------------------------
    def read_from_file(self, f):
        content = f.read().split()
        if len(content) == 2:
            return int(content[0]), int(content[1])
        else:
            self._log.error(f"Invalid stats content in: {f.name}")
        return 0,0

    # --------------------------------------------------------------------------
    def write_to_file(self, f, hits, misses, hit):
        if hit:
            hits += 1
        else:
            misses += 1
        f.write(f"{hits} {misses}\n")

    # --------------------------------------------------------------------------
    def update(self, digest, hit):
        try:
            file = self.stats_file(digest)
            mkdir_p(os.path.dirname(file))
            with MultiprocessLock(file + ".lock") as _:
                try:
                    if os.path.isfile(file):
                        with open(file, 'r+') as fh:
                            hits, misses = self.read_from_file(fh)
                            fh.seek(0)
                            self.write_to_file(fh, hits, misses, hit)
                            fh.truncate()
                    else:
                        with open(file, 'w') as fh:
                            self.write_to_file(fh, 0, 0, hit)
                except IOError as e:
                    self._log.error(f"Error writing to file: {e}")
        except Exception as e:
            traceback.print_exc(file=sys.stdout)
            raise

    # --------------------------------------------------------------------------
    def clear(self):
        for i in range(0, 256):
            digest = f'{i:x}'
            file = self.stats_file(digest)
            if os.path.isfile(file):
                os.unlink(file)

# ------------------------------------------------------------------------------
class ClangTidyLocalCache:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts):
        self._log = log
        self._opts = opts
        self._hash_regex = re.compile(r'^[0-9a-f]{38}$')

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        path = self._make_path(digest)
        if os.path.isfile(path):
            os.utime(path, None)
            return True

        return False

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        path = self._make_path(digest)
        if os.path.isfile(path):
            os.utime(path, None)
            with open(path, "rb") as stream:
                return stream.read()
        else:
            return None

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        p = self._make_path(digest)
        mkdir_p(os.path.dirname(p))
        open(p, "w").close()

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        p = self._make_path(digest)
        mkdir_p(os.path.dirname(p))
        with open(p, "wb") as stream:
            stream.write(data)

    # --------------------------------------------------------------------------
    def _list_cached_files(self, options, prefix):
        for root, dirs, files in os.walk(prefix):
            for prefix in dirs:
                for filename in self._list_cached_files(options, prefix):
                    if self._hash_regex.match(filename):
                        yield root, prefix, filename
            for filename in files:
                if self._hash_regex.match(filename):
                    yield root, prefix, filename

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        hash_count = sum(1 for x in self._list_cached_files(options, options.cache_dir))
        return {"cached_count": hash_count}

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        pass

    # --------------------------------------------------------------------------
    def _make_path(self, digest):
        return os.path.join(self._opts.cache_dir, digest[:2], digest[2:])

# ------------------------------------------------------------------------------
class ClangTidyRedisCache:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts: ClangTidyCacheOpts):
        self._log = log
        self._opts = opts
        assert redis
        self._cli = redis.Redis(
            host=opts.redis_host(),
            port=opts.redis_port(),
            db=opts.redis_db(),
            username=opts.redis_username(),
            password=opts.redis_password(),
            socket_connect_timeout=opts.redis_connect_timeout(),
            socket_timeout=opts.redis_socket_timeout(),
            # the two settings below are used to avoid sending any commands to the Redis
            # server other than AUTH, GET, and SET (to let the ctcache operate with a
            # server configuration giving only minimal permissions to the given user)
            lib_name=None,
            lib_version=None)
        self._namespace = opts.redis_namespace()

    # --------------------------------------------------------------------------
    def _get_key_from_digest(self, digest) -> str:
        return self._namespace + digest

    # --------------------------------------------------------------------------
    def is_cached(self, digest) -> bool:
        n_digest = self._get_key_from_digest(digest)
        return self._cli.get(n_digest) is not None

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        n_digest = self._get_key_from_digest(digest)
        data = self._cli.get(n_digest)
        ttl = self._opts.redis_cache_ttl()
        if not self._opts.redis_read_only() and data is not None and ttl is not None:
            # try to extend TTL on cache hits
            self._cli.expire(n_digest, ttl, xx=True)
        return data

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        self.store_in_cache_with_data(digest, bytes())

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        if self._opts.redis_read_only():
            return
        n_digest = self._get_key_from_digest(digest)
        self._cli.set(n_digest, data, ex=self._opts.redis_cache_ttl())

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        # TODO
        pass

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        # TODO
        pass

# ------------------------------------------------------------------------------
class ClangTidyS3Cache:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts):
        from boto3 import client
        from botocore.exceptions import ClientError
        from botocore.config import Config
        from botocore.session import UNSIGNED

        self._ClientError = ClientError
        self._log = log
        self._opts = opts
        if self._opts.s3_no_credentials():
            self._client = client("s3", config=Config(signature_version=UNSIGNED))
        else:
            self._client = client("s3")
        self._bucket = opts.s3_bucket()
        self._bucket_folder = opts.s3_bucket_folder()

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        try:
            path = self._make_path(digest)
            self._client.get_object(Bucket=self._bucket, Key=path)
        except self._ClientError as e:
            if e.response['Error']['Code'] == "NoSuchKey":
                return False

            self._log.error("Error calling S3:get_object %s", str(e))
            raise

        return True

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        # TODO
        return None

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        if self._opts.s3_no_credentials() or self._opts.s3_read_only():
            return
        try:
            path = self._make_path(digest)
            self._client.put_object(Bucket=self._bucket, Key=path, Body=digest)
        except self._ClientError as e:
            self._log.error("Error calling S3:put_object {}".format(str(e)))
            raise

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        # TODO
        pass

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        # TODO
        pass

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        # TODO
        pass

    # --------------------------------------------------------------------------
    def _make_path(self, digest):
        return os.path.join(self._bucket_folder, digest[:2], digest[2:])

# ------------------------------------------------------------------------------
class ClangTidyGcsCache:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts):
        import google.cloud.storage as gcs

        self._log = log
        self._opts = opts
        if self._opts.gcs_no_credentials():
            self._client = gcs.Client.create_anonymous_client()
        else:
            self._client = gcs.Client()
        self._bucket = self._client.bucket(opts.gcs_bucket())
        self._bucket_folder = opts.gcs_bucket_folder()

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        try:
            path = self._make_path(digest)
            blob = self._bucket.blob(path)
            t = blob.exists()
            return t
        except Exception as e:
            self._log.error("Error calling GCS:blob.exists {}".format(str(e)))
            raise

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        try:
            path = self._make_path(digest)
            blob = self._bucket.blob(path)
            return blob.download_as_bytes()
        except Exception as e:
            return None

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        if self._opts.gcs_no_credentials() or self._opts.gcs_read_only():
            return
        try:
            path = self._make_path(digest)
            blob = self._bucket.blob(path)
            blob.upload_from_string(digest)
        except Exception as e:
            self._log.error(
                "Error calling GCS:blob.upload_from_string {}".format(str(e)))
            raise

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        if self._opts.gcs_no_credentials() or self._opts.gcs_read_only():
            return
        try:
            path = self._make_path(digest)
            blob = self._bucket.blob(path)
            blob.upload_from_string(data, content_type="application/octet-stream")
        except Exception as e:
            self._log.error(
                "Error calling GCS:blob.upload_from_string {}".format(str(e)))
            raise

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        # TODO
        pass

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        # TODO
        pass

    # --------------------------------------------------------------------------
    def _make_path(self, digest):
        return os.path.join(self._bucket_folder, digest[:2], digest[2:])

# ------------------------------------------------------------------------------
class ClangTidyMultiCache:
    # --------------------------------------------------------------------------
    def __init__(self, log, caches):
        self._log = log
        self._caches = caches

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        for cache in self._caches:
            if cache.is_cached(digest):
                return True

        return False

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        for cache in self._caches:
            data = cache.get_cache_data(digest)
            if data is not None:
                return data

        return None

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        for cache in self._caches:
            cache.store_in_cache(digest)

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        for cache in self._caches:
            cache.store_in_cache_with_data(digest, data)

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        for cache in self._caches:
            stats = cache.query_stats(options)
            if stats:
                return stats

        return {}

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        for cache in self._caches:
            cache.clear_stats(options)

# ------------------------------------------------------------------------------
class ClangTidyCacheWithStats:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts, cache, stats):
        self._log = log
        self._opts = opts
        self._cache = cache
        self._stats = stats

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        res = self._cache.is_cached(digest)
        if self._stats:
            self._stats.update(digest, res)
        return res

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        res = self._cache.get_cache_data(digest)
        if self._stats:
            self._stats.update(digest, res is not None)
        return res

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        self._cache.store_in_cache(digest)

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        self._cache.store_in_cache_with_data(digest, data)

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        stats = self._cache.query_stats(options)
        if stats is None:
            stats = {}

        if self._stats:
            hits, misses = self._stats.read()
            total = hits + misses
            stats["hit_count"] = hits
            stats["miss_count"] = misses
            stats["hit_rate"] = hits/total if total else 0
            stats["miss_rate"] = misses/total if total else 0

        return stats

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        self._cache.clear_stats(options)
        if self._stats:
            self._stats.clear()

# ------------------------------------------------------------------------------
class ClangTidyCache:
    # --------------------------------------------------------------------------
    def __init__(self, log, opts: ClangTidyCacheOpts):
        self._log = log
        self._opts = opts
        self._local = None
        self._remote = None

        caches = []

        if opts.has_host():
            caches.append(ClangTidyServerCache(log, opts))

        if opts.has_redis_host() and redis:
            caches.append(ClangTidyRedisCache(log, opts))

        if opts.has_s3():
            caches.append(ClangTidyS3Cache(log, opts))

        if opts.has_gcs():
            caches.append(ClangTidyGcsCache(log, opts))

        if not caches or opts.cache_locally():
            local = ClangTidyLocalCache(log, opts)
            self._local = self._wrap_with_stats(local, "stats")

        if caches:
            remote = ClangTidyMultiCache(log, caches)
            self._remote = self._wrap_with_stats(remote, "remote_stats")

    # --------------------------------------------------------------------------
    def _wrap_with_stats(self, cache, name):
        if not self._opts.no_local_stats():
            stats = ClangTidyCacheStats(self._log, self._opts, name)
            return ClangTidyCacheWithStats(self._log, self._opts, cache, stats)
        return cache

    # --------------------------------------------------------------------------
    def is_cached(self, digest):
        if self._local:
            if self._local.is_cached(digest):
                return True

        if self._remote:
            if self._remote.is_cached(digest):
                if self.should_writeback():
                    self._local.store_in_cache(digest)
                return True

        return False

    # --------------------------------------------------------------------------
    def get_cache_data(self, digest) -> Optional[bytes]:
        if self._local:
            data = self._local.get_cache_data(digest)
            if data is not None:
                return data

        if self._remote:
            data = self._remote.get_cache_data(digest)
            if data is not None:
                if self.should_writeback():
                    self._local.store_in_cache_with_data(digest, data)
                return data

        return None

    # --------------------------------------------------------------------------
    def store_in_cache(self, digest):
        if self._local:
            self._local.store_in_cache(digest)

        if self._remote:
            self._remote.store_in_cache(digest)

    # --------------------------------------------------------------------------
    def store_in_cache_with_data(self, digest, data: bytes):
        if self._local:
            self._local.store_in_cache_with_data(digest, data)

        if self._remote:
            self._remote.store_in_cache_with_data(digest, data)

    # --------------------------------------------------------------------------
    def query_stats(self, options):
        stats = {}

        if self._local:
            stats["local"] = self._local.query_stats(options)

        if self._remote:
            stats["remote"] = self._remote.query_stats(options)

        return stats

    # --------------------------------------------------------------------------
    def clear_stats(self, options):
        if self._local:
            self._local.clear_stats(options)

        if self._remote:
            self._remote.clear_stats(options)

    # --------------------------------------------------------------------------
    def should_writeback(self):
        return self._local is not None and not self._opts.no_local_writeback()

# ------------------------------------------------------------------------------
def remove_matching_line(byte_stream, pattern):
    text = byte_stream.decode("utf-8")
    lines = text.split("\n")
    regex = re.compile(pattern)
    filtered_lines = [line for line in lines if not regex.search(line)]
    filtered_text = "\n".join(filtered_lines)
    return filtered_text.encode("utf-8")

# ------------------------------------------------------------------------------
def hash_inputs(log, opts):
    ct_args = opts.clang_tidy_args()
    co_args = opts.compiler_args()

    if not ct_args and not co_args:
        return None

    def _is_src_ext(s):
        exts = [".cppm", ".cpp", ".c", ".cc", ".h", ".hpp", ".cxx"]
        return any(s.lower().endswith(ext) for ext in exts)

    result = ClangTidyCacheHash(opts)

    # --- Source file content (potentially pre-processed)
    if len(co_args) == 0:
        for arg in ct_args[1:]:
            if os.path.exists(arg) and _is_src_ext(arg):
                with open(arg, "rb") as srcfd:
                    src_data_binary = srcfd.read()
                    if opts.strip_src():
                        src_data = src_data_binary.decode(encoding="utf-8")
                        src_data = opts.strip_paths(src_data)
                        src_data_binary = src_data.encode("utf-8")
                    result.update(src_data_binary)
    else:
        # Execute the compiler command defined by the compiler arguments. At this
        # point if we have compiler arguments with expect that it defines a valid
        # command to get the pre-processed output.
        # If we have a valid output this gets added to the hash.
        proc = subprocess.Popen(
            co_args,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        stdout, stderr = proc.communicate()
        if opts.running_on_msvc() or opts.running_on_clang_cl():
            if proc.returncode != 0:
                return None
        else:
            if stderr:
                log.error(f"Error executing compile command: #{co_args}.\n#{stderr}")
                return None

        if opts.strip_src():
            stdout_str = stdout.decode(encoding="utf-8")
            stdout_str = opts.strip_paths(stdout_str)
            stdout = stdout_str.encode("utf-8")

        result.update(stdout)

    # --- Config Contents ------------------------------------------------------
    # (as obtained by running clang-tidy with --dump-config flag)

    ct_args_flags = [ ct_args[0] ]
    source_files = []

    for arg in ct_args[1:]:
        if os.path.exists(arg) and _is_src_ext(arg):
            source_files.append(os.path.normpath(os.path.realpath(arg)))
        else:
            ct_args_flags.append(arg)

    for source_file in sorted(source_files):
        ct_dump_cfg_source_file = ct_args_flags + [ "--dump-config",  source_file ]
        proc = subprocess.Popen(
            ct_dump_cfg_source_file,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        stdout, stderr = proc.communicate()
        if opts.exclude_user_config():
            stdout = remove_matching_line(stdout, "User:.*$")
            stdout = remove_matching_line(stdout, "HeaderFilterRegex:.*$")

        if (proc.returncode == 0) and (len(stdout) > 0):
            result.update(stdout)
        else:
            msg = f"Failed dumping the clang-tidy config with <{' '.join(ct_dump_cfg_source_file)}>"
            raise RuntimeError(msg)

    # --- Clang-Tidy and Compiler Args -----------------------------------------

    def _omit_after(args, excl):
        omit_next = False
        for arg in args:
            omit_this = arg in excl
            if not omit_this and not omit_next:
                yield arg
            omit_next = omit_this

    ct_args = list(_omit_after(ct_args, ["-export-fixes"]))

    for chunk in sorted(set(opts.adjust_chunk(arg) for arg in ct_args[1:])):
        if not opts.exclude_hash(chunk):
            result.update(chunk)

    for chunk in sorted(set(opts.adjust_chunk(arg) for arg in co_args[1:])):
        if not opts.exclude_hash(chunk):
            result.update(chunk)

    return result.hexdigest()

# ------------------------------------------------------------------------------
def print_usage():
    print("Usage: clang-tidy-cache /path/to/real/clang-tidy [[cache-options] --] <clang-tidy-options>")
# ------------------------------------------------------------------------------
def print_stats(log, opts, raw):
    def _format_bytes(s):
        if s < 10000:
            return "%d B" % (s)
        if s < 10000000:
            return "%d kB" % (s / 1000)
        return "%d MB" % (s / 1000000)

    def _format_time(s):
        if s < 60:
            return "%d seconds" % (s)
        if s < 3600:
            return "%d minutes %d seconds" % (s / 60, s % 60)
        if s < 86400:
            return "%d hours %d minutes" % (s / 3600, (s / 60) % 60)
        if s < 604800:
            return "%d days %d hours" % (s / 86400, (s / 3600) % 24)
        if int(s / 86400) % 7 == 0:
            return "%d weeks" % (s / 604800)
        return "%d weeks %d days" % (s / 604800, (s / 86400) % 7)

    cache = ClangTidyCache(log, opts)
    stats = cache.query_stats(opts)

    if raw:
        print(json.dumps(stats))
        return

    entries = [
        ("Server host", lambda o, s: o.rest_host()),
        ("Server port", lambda o, s: "%d" % o.rest_port()),
        ("Long-term hit rate", lambda o, s: "%.1f %%" % (s["remote"]["total_hit_rate"] * 100.0)),
        ("Hit rate", lambda o, s: "%.1f %%" % (s["remote"]["hit_rate"] * 100.0)),
        ("Hit count", lambda o, s: "%d" % s["remote"]["hit_count"]),
        ("Miss count", lambda o, s: "%d" % s["remote"]["miss_count"]),
        ("Miss rate", lambda o, s: "%.1f %%" % (s["remote"]["miss_rate"] * 100.0)),
        ("Max hash age", lambda o, s: "%d days" % max(int(k) for k in s["remote"]["age_days_histogram"])),
        ("Max hash hits", lambda o, s: "%d" % max(int(k) for k in s["remote"]["hit_count_histogram"])),
        ("Cache size", lambda o, s: _format_bytes(s["remote"]["saved_size_bytes"])),
        ("Cached hashes", lambda o, s: "%d" % s["remote"]["cached_count"]),
        ("Cleaned hashes", lambda o, s: "%d" % s["remote"]["cleaned_count"]),
        ("Cleaned ago", lambda o, s: _format_time(s["remote"]["cleaned_seconds_ago"])),
        ("Saved ago", lambda o, s: _format_time(s["remote"]["saved_seconds_ago"])),
        ("Uptime", lambda o, s: _format_time(s["remote"]["uptime_seconds"])),
        ("Hit rate (local)", lambda o, s: "%.1f %%" % (s["local"]["hit_rate"] * 100.0)),
        ("Hit count (local)", lambda o, s: "%d" % s["local"]["hit_count"]),
        ("Miss count (local)", lambda o, s: "%d" % s["local"]["miss_count"]),
        ("Miss rate (local)", lambda o, s: "%.1f %%" % (s["local"]["miss_rate"] * 100.0)),
        ("Cached hashes (local)", lambda o, s: "%d" % s["local"]["cached_count"])
    ]

    max_len = max(len(e[0]) for e in entries)
    for label, fmtfunc in entries:
        padding = " " * (max_len-len(label))
        try:
            print(label+":", padding, fmtfunc(opts, stats))
        except:
            print(label+":", padding, "N/A")

# ------------------------------------------------------------------------------
def clear_stats(log, opts):
    "Clears the cache statistics."
    cache = ClangTidyCache(log, opts)
    cache.clear_stats(opts)

# ------------------------------------------------------------------------------
def run_clang_tidy_cached(log, opts):
    cache = ClangTidyCache(log, opts)
    digest = None
    try:
        digest = hash_inputs(log, opts)
        if digest and opts.save_output():
            data = cache.get_cache_data(digest)
            if data is not None:
                returncode = int(data[0])
                sys.stdout.write(data[1:].decode("utf8"))
                return returncode
        elif digest and cache.is_cached(digest):
            return 0
        else:
            pass
    except Exception as error:
        log.error(str(error))

    proc = subprocess.Popen(
        opts.original_args(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = proc.communicate()
    sys.stdout.write(stdout.decode("utf8"))
    sys.stderr.write(stderr.decode("utf8"))

    tidy_success = True
    if proc.returncode != 0:
        tidy_success = False

    if stdout and not opts.ignore_output():
        tidy_success = False

    # saving the result even in case clang-tidy wasn't successful is only meaningful
    # if the output is actually stored. Only then the exit code can be retained
    # (as the first byte in the corresponding key's value)
    save_even_without_success = opts.save_all() and opts.save_output()

    if (tidy_success or save_even_without_success) and digest:
        try:
            if opts.save_output():
                returncode_and_ct_output = bytes([proc.returncode]) + stdout
                cache.store_in_cache_with_data(digest, returncode_and_ct_output)
            else:
                cache.store_in_cache(digest)
        except Exception as error:
            log.error(str(error))

    return proc.returncode

# ------------------------------------------------------------------------------
def main():
    log = logging.getLogger(os.path.basename(__file__))
    log.setLevel(logging.WARNING)
    debug = False
    opts = None
    try:
        opts = ClangTidyCacheOpts(log, sys.argv[1:])
        debug = opts.debug_enabled()
        if opts.should_print_usage():
            print_usage()
        elif opts.should_print_dir():
            print(opts.cache_dir)
        elif opts.should_remove_dir():
            try:
                shutil.rmtree(opts.cache_dir)
            except FileNotFoundError:
                pass
        elif opts.should_print_stats():
            print_stats(log, opts, False)
        elif opts.should_print_stats_raw():
            print_stats(log, opts, True)
        elif opts.should_zero_stats():
            clear_stats(log, opts)
        else:
            return run_clang_tidy_cached(log, opts)
        return 0
    except Exception as error:
        if debug:
            log.error("Options: %s", repr(opts))
            raise
        log.error("%s: %s", str(type(error)), repr(error))
        return 1

# ------------------------------------------------------------------------------
if __name__ == "__main__":
    sys.exit(main())
