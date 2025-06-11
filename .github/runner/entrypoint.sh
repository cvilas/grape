#!/bin/bash
set -e

# Fix permissions for mounted work directory
chown -R runner:runner /home/runner/_work || true

# Drop privileges to 'runner' user and exec the command
exec gosu runner "$@"
