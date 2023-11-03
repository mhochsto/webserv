#!/bin/bash

TESTFILE=".testcases"

RESET='\033[0m'
RED='\033[0;31m'

if [ -n "$1" ]; then
  Domain="$1"
else
    Domain="http://localhost:7700"
    echo -e "Using Default Domain: $Domain\n"
fi

if [ ! -f "$TESTFILE" ]; then
  echo "File not found: $TESTFILE"
  exit 1
fi

while IFS= read -r description; do
    read -r command
    if [ -z "$description" ] || [ -z "$command" ]; then
        continue
    fi
    test_command="$(echo "$command" | sed "s|Domain\b|$Domain|g")"
    echo -e "$RED$description"
    echo -e "$test_command\n"
    echo -e "++++ Output start ++++  $RESET"
    eval "$test_command"
    echo -e "\n$RED ++++ output end ++++ \n$RESET"
done < "$TESTFILE"
