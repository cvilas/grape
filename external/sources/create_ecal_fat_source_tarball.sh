#!/bin/bash
# Script to create a fat source archive for the eCAL repository,
# including patching git-get-revision-information.cmake with version data.
# Falls back to version 0.0.0 if commit info/tag is not available.

set -e

# 1. Verify cwd is repo root (contains .git, etc.)
if ! [ -d .git ]; then
  echo "Error: Must run from the root of the eCAL repository."
  exit 1
fi

# 2. Submodule update (if not already)
git submodule update --init --recursive

# 3. Create clean directory
WORKSPACE="$(pwd)"
CLEANED_DIR="${WORKSPACE}/cleaned"
SRC_OUT="${CLEANED_DIR}/ecal"
ARCHIVE_NAME="${CLEANED_DIR}/ecal-fat-source.tar.gz"

rm -rf "$CLEANED_DIR"
mkdir -p "$SRC_OUT"

rsync \
  --recursive \
  --delete \
  --delete-excluded \
  --exclude=".git" \
  --exclude="*.git" \
  --stats \
  "$WORKSPACE"/ \
  "$SRC_OUT"/

# 4. Patch git-get-revision-information.cmake
cd "$WORKSPACE"

# Try to get tag-based description
git_version_complete=$(git describe --tags --dirty 2>/dev/null || echo "")
git_commit_date=$(git show -s --format=%ci 2>/dev/null || echo "")

if [[ -z "$git_version_complete" || "$git_version_complete" == "fatal:"* ]]; then
  # Fallback to "v0.0.0" and today's date
  git_version_complete="v6.1.0"
  ecal_version_major="6"
  ecal_version_minor="1"
  ecal_version_patch="0"
  git_commit_date="$(date -Iseconds)"
else
  IFS='.'
  read -ra ecal_version_array <<< "${git_version_complete:1}"

  ecal_version_major="${ecal_version_array[0]}"
  ecal_version_minor="${ecal_version_array[1]}"

  # Remove -nightly or +nightly etc. from patch version
  IFS='-'
  read -ra ecal_patch_array <<< "${ecal_version_array[2]}"
  IFS='+'
  read -ra ecal_patch_array <<< "${ecal_patch_array[0]}"

  ecal_version_patch="${ecal_patch_array[0]}"
fi

# Patch file location
PATCH_PATH="${SRC_OUT}/thirdparty/cmakefunctions/cmake_functions/git/git_revision_information.cmake"
mkdir -p "$(dirname "$PATCH_PATH")"

cat > "$PATCH_PATH" <<EOF
function (git_revision_information)
  set(GIT_REVISION_MAJOR ${ecal_version_major}   PARENT_SCOPE)
  set(GIT_REVISION_MAYOR ${ecal_version_major}   PARENT_SCOPE)
  set(GIT_REVISION_MINOR ${ecal_version_minor}   PARENT_SCOPE)
  set(GIT_REVISION_PATCH ${ecal_version_patch}  PARENT_SCOPE)
  set(GIT_DESCRIBE_TAG   "${git_version_complete}"      PARENT_SCOPE)
  set(GIT_REVISION_DATE  "${git_commit_date}" PARENT_SCOPE)
  set(eCAL_BUILD_DATE    "${git_commit_date}" PARENT_SCOPE)
endfunction (git_revision_information)
EOF

# 5. Create fat archive
cd "$CLEANED_DIR"
tar -czvf "$ARCHIVE_NAME" ecal

echo "Fat archive created at: $ARCHIVE_NAME"
