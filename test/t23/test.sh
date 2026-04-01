#!/bin/env bash

# change to script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR" || { echo "Failed to change directory to $SCRIPT_DIR"; exit 1; }

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

for file in mail_html.txt mail_plain.txt mail_subject.txt; do
    if [ -f "$file" ]; then
        if ! rm "$file"; then
            echo -e "${RED}✗ Failed to remove $file${NC}"
            exit 1
        fi
    fi
done

# Run the analyzer
if ! ../../lotr_analyzer -test mail; then
    echo -e "${RED}✗ lotr_analyzer command failed${NC}"
    exit 1
fi

# Check if files exist
for file in mail_plain.txt mail_subject.txt; do
    if ! [ -f "$file" ]; then
        echo -e "${RED}✗ $file not produced${NC}"
        exit 1
    fi
done

# Compare
failed=0
for pair in "mail_plain.txt mail_plain.expected.txt" "mail_subject.txt mail_subject.expected.txt"; do
    set -- $pair
    if diff "$1" "$2" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ $1 matches expected${NC}"
    else
        echo -e "${RED}✗ $1 differs from expected${NC}"
        failed=1
    fi
done

if [ $failed -eq 1 ]; then
    exit 1
fi
