#!/bin/bash

# Shell script to execute Docker container with volume mapping
# Maps current directory to /workspace in container

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CURRENT_DIR="$(pwd)"
IMAGE_NAME="lotr-analyzer-container"
CONTAINER_NAME="lotr-analyzer-build"

echo "Building Docker image..."
docker build -t "$IMAGE_NAME" "$SCRIPT_DIR"

echo "Running container with volume mapping..."
echo "  Image: $IMAGE_NAME"
echo "  Container: $CONTAINER_NAME"
echo "  Local directory: $CURRENT_DIR"
echo "  Container mount: /workspace"

#echo "cmd: ${@}"
docker run --rm \
  --name "$CONTAINER_NAME" \
  -v "$CURRENT_DIR:/workspace" \
  -w /workspace \
  "$IMAGE_NAME" \
  /bin/bash -c "$*"

echo "Container execution completed."
