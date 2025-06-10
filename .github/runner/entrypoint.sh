#!/bin/bash
set -e

# Fix permissions for mounted work directory
chown -R runner:runner /home/runner/_work || true

exec "$@"
