#!/usr/bin/env bash

# change to script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR" || { echo "Failed to change directory to $SCRIPT_DIR"; exit 1; }

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

failed=0

for dir in */; do
    dir=${dir%/}  # Remove trailing slash
    if [ -f "$dir/test.sh" ]; then
        echo -e "Executing test in subdirectory: ${YELLOW}$dir${NC}"
        if ./$dir/test.sh; then
            echo -e "${GREEN}✓ $dir test passed${NC}"
        else
            echo -e "${RED}✗ $dir test failed${NC}"
            failed=1
        fi
        echo # Add a blank line for better readability
    fi
done

if [ $failed -eq 1 ]; then
    exit 1
fi


