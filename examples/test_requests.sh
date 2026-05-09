#!/bin/bash

# POST Request Testing Script
# This script demonstrates various ways to test POST requests to the webserver

SERVER="http://127.0.0.1:2222"

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Webserver POST Request Tests ===${NC}\n"

# Test 1: Simple POST request
echo -e "${GREEN}Test 1: Simple POST request${NC}"
echo "Sending: 'Hello from curl'"
curl -X POST --data "Hello from curl" "$SERVER/uploads/hello.txt" -v
echo -e "\n"

# Test 2: POST with file content
echo -e "${GREEN}Test 2: POST with file content${NC}"
echo "Creating test file..."
echo "This is test content from a file" > /tmp/test_content.txt
curl -X POST --data-binary @/tmp/test_content.txt "$SERVER/uploads/from_file.txt" -v
echo -e "\n"

# Test 3: POST with JSON
echo -e "${GREEN}Test 3: POST with JSON content${NC}"
curl -X POST \
  -H "Content-Type: application/json" \
  --data '{"name":"webserver","test":true}' \
  "$SERVER/uploads/data.json" -v
echo -e "\n"

# Test 4: POST with multiline content
echo -e "${GREEN}Test 4: POST with multiline content${NC}"
curl -X POST --data "Line 1: First line
Line 2: Second line
Line 3: Third line" "$SERVER/uploads/multiline.txt" -v
echo -e "\n"

# Test 5: POST to nested path (requires directory to exist)
echo -e "${GREEN}Test 5: POST to nested path${NC}"
curl --data "Nested file content" "$SERVER/uploads/nested/file.txt" -v 
echo -e "\n"

# Test 6: GET request to verify POST created file
echo -e "${GREEN}Test 6: GET to verify POST created file${NC}"
curl "$SERVER/uploads/hello.txt" -v
echo -e "\n"

# Test 7: POST with form data
echo -e "${GREEN}Test 7: POST with form data${NC}"
curl -X POST \
  -F "field1=value1" \
  -F "field2=value2" \
  "$SERVER/uploads/form_data.txt" -v
echo -e "\n"

# Test 8: POST with empty body
echo -e "${GREEN}Test 8: POST with empty body${NC}"
curl -X POST "$SERVER/uploads/empty.txt" -v
echo -e "\n"

# Test 9: POST large content
echo -e "${GREEN}Test 9: POST with large content${NC}"
python3 -c "print('x' * 1000)" | curl -X POST --data-binary @- "$SERVER/uploads/large.txt" -v
echo -e "\n"

# Test 10: POST binary content
echo -e "${GREEN}Test 10: POST binary content${NC}"
dd if=/dev/urandom bs=100 count=1 2>/dev/null | curl -X POST --data-binary @- "$SERVER/uploads/binary.bin" -v
echo -e "\n"

echo -e "${BLUE}=== Tests Complete ===${NC}"
echo "Check /root/uploads/ directory for created files"
