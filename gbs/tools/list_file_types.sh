#!/bin/bash
# List all unique file extensions in the specified directory and below.

# Check if a directory path is provided
if [ -z "$1" ]; then
  echo "Usage: $0 <directory_path>"
  exit 1
fi

# Check if the provided path is a directory
if [ ! -d "$1" ]; then
    echo "Error: '$1' is not a directory."
    exit 1
fi

TARGET_DIR="$1"

echo "Unique file types in '$TARGET_DIR':"

# - Find all files (not directories) in the specified directory.
# - For each file, extract the extension.
# - Sort the extensions.
# - Get a unique list of extensions.
# - Filter out empty lines.
find "$TARGET_DIR" -type f | sed -n 's/.*\.//p' | sort -u | grep -v '^$'
