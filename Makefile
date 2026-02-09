# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -std=c++17
LDFLAGS =

# Directories
BUILD_DIR = build
UTILS_DIR = utils
SOCKETS_DIR = sockets
CLIENT_DIR = client

# Output files
UTILS_BIN = $(BUILD_DIR)/test_utils
SOCKETS_BIN = $(BUILD_DIR)/test_sockets
CLIENT_BIN = $(BUILD_DIR)/test_client
CLIENT_REQUEST_BIN = $(BUILD_DIR)/test_request

# PHONY targets
.PHONY: all clean utils sockets client test_utils test_sockets test_client test_request request help

# Default target
all: utils sockets client

help:
	@echo "Available targets:"
	@echo "  make utils          - Build utils module"
	@echo "  make sockets        - Build sockets module"
	@echo "  make client         - Build client module"
	@echo "  make request        - Build request parser test"
	@echo "  make test_utils     - Test utils module"
	@echo "  make test_sockets   - Test sockets module"
	@echo "  make test_client    - Test client module"
	@echo "  make test_request   - Run request parser test (listens on port 2222)"
	@echo "  make all            - Build all modules"
	@echo "  make clean          - Clean build artifacts"
	@echo "  make clean_all      - Clean all modules"

# Utils module
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

utils: $(BUILD_DIR) $(UTILS_BIN)

$(UTILS_BIN): $(UTILS_DIR)/main.cpp $(UTILS_DIR)/option/Option.hpp
	@echo "Building utils module..."
	$(CXX) $(CXXFLAGS) -o $@ $(UTILS_DIR)/main.cpp $(LDFLAGS)
	@echo "✓ Utils module built"

test_utils: $(UTILS_BIN)
	@echo "Testing utils module..."
	@./$(UTILS_BIN)
	@echo "✓ Utils tests passed"

# Sockets module
sockets: $(BUILD_DIR) $(SOCKETS_BIN)

$(SOCKETS_BIN): $(SOCKETS_DIR)/main.cpp $(SOCKETS_DIR)/*.cpp $(SOCKETS_DIR)/*.hpp $(CLIENT_DIR)/*.cpp $(CLIENT_DIR)/*.hpp
	@echo "Building sockets module..."
	$(CXX) $(CXXFLAGS) -o $@ $(SOCKETS_DIR)/main.cpp $(SOCKETS_DIR)/*.cpp $(CLIENT_DIR)/*.cpp $(LDFLAGS)
	@echo "✓ Sockets module built"

test_sockets: $(SOCKETS_BIN)
	@echo "Testing sockets module..."
	@./$(SOCKETS_BIN)
	@echo "✓ Sockets tests passed"

# Client module
client: $(BUILD_DIR) $(CLIENT_BIN)

$(CLIENT_BIN): $(CLIENT_DIR)/Client.cpp $(CLIENT_DIR)/Request.cpp
	@echo "Building client module..."
	$(CXX) $(CXXFLAGS) -c -o $(BUILD_DIR)/Client.o $(CLIENT_DIR)/Client.cpp
	$(CXX) $(CXXFLAGS) -c -o $(BUILD_DIR)/Request.o $(CLIENT_DIR)/Request.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(BUILD_DIR)/Client.o $(BUILD_DIR)/Request.o $(LDFLAGS)
	@echo "✓ Client module built"

test_client: $(CLIENT_BIN)
	@echo "Testing client module..."
	@./$(CLIENT_BIN)
	@echo "✓ Client tests passed"

# Request parser test
request: $(BUILD_DIR) $(CLIENT_REQUEST_BIN)

$(CLIENT_REQUEST_BIN): $(CLIENT_DIR)/main_request.cpp $(CLIENT_DIR)/Request.cpp $(CLIENT_DIR)/Request.hpp
	@echo "Building request parser test..."
	$(CXX) $(CXXFLAGS) -o $@ $(CLIENT_DIR)/main_request.cpp $(CLIENT_DIR)/Request.cpp $(UTILS_DIR)/str_slice/StrSlice.cpp $(LDFLAGS)
	@echo "✓ Request parser test built"

test_request: $(CLIENT_REQUEST_BIN)
	@echo "Starting request parser test on port 2222..."
	@echo "In another terminal, test with: curl http://127.0.0.1:2222/"
	@./$(CLIENT_REQUEST_BIN)

# Clean targets
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@echo "✓ Clean complete"

clean_all: clean
	@find . -name "*.o" -delete
	@find . -name "a.out" -delete
	@echo "✓ Full clean complete"

# Run all tests
test: test_utils test_sockets test_client
	@echo "✓ All tests passed!"
