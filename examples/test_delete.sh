#!/bin/bash

# DELETE Request Testing Script
# This script deletes the files created by the POST tests in test_requests.sh
# Usage: ./examples/test_delete.sh

SERVER="http://127.0.0.1:2222"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Webserver DELETE Request Tests ===${NC}\n"

# List of files to delete (matching those from test_requests.sh)
FILES=(
  "hello.txt"
  "from_file.txt"
  "data.json"
  "multiline.txt"
  "nested/file.txt"
  "form_data.txt"
  "empty.txt"
  "large.txt"
  "binary.bin"
)

for FILE in "${FILES[@]}"; do
  echo -e "${GREEN}Test: DELETE $FILE${NC}"
  curl -X DELETE "$SERVER/uploads/$FILE" -v
  echo -e "\n"
done

echo -e "${BLUE}=== Delete Tests Complete ===${NC}"
