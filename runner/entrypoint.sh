#!/bin/bash
set -e

# Fix permissions for mounted work directory and tool cache
chown -R runner:runner /home/runner/_work || true
chown -R runner:runner /home/runner/_tool || true

exec "$@"