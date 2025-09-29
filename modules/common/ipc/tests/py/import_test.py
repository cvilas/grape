#!/usr/bin/env python3
"""Test that the Python bindings can be imported."""
import sys

try:
    from grape_ipc_py import Config, init, RawPublisher, RawSubscriber, ok
    print("SUCCESS: All imports successful")
    sys.exit(0)
except ImportError as e:
    print(f"FAILED: Import error: {e}")
    sys.exit(1)
except Exception as e:
    print(f"FAILED: Unexpected error: {e}")
    sys.exit(1)