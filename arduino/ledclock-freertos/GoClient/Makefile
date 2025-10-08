# Makefile for generating Go code from ../docs/openapi.json

OPENAPI_FILE = ../docs/openapi.json
OUTPUT_DIR = gen
PACKAGE_NAME = ledclock
OAPI_CODEGEN = $(shell which oapi-codegen 2>/dev/null)

# Default target
.PHONY: all
all: help

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  make install   - Install oapi-codegen if missing"
	@echo "  make client    - Generate Go client code"
	@echo "  make server    - Generate Go server stubs"
	@echo "  make types     - Generate Go type definitions"
	@echo "  make allgen    - Generate client, server, and types"
	@echo "  make clean     - Remove generated files"

# --- Install oapi-codegen if missing ---
.PHONY: install
install:
ifeq ($(OAPI_CODEGEN),)
	@echo "Installing oapi-codegen..."
	go install github.com/deepmap/oapi-codegen/v2/cmd/oapi-codegen@latest
else
	@echo "oapi-codegen already installed: $(OAPI_CODEGEN)"
endif

# --- Generate Go client ---
.PHONY: client
client: install
	@echo "Generating Go client..."
	@mkdir -p $(OUTPUT_DIR)
	oapi-codegen -generate types,client -o $(OUTPUT_DIR)/$(PACKAGE_NAME)_client.gen.go -package $(PACKAGE_NAME) $(OPENAPI_FILE)
	go fmt ./$(OUTPUT_DIR)

# --- Generate Go server stubs ---
.PHONY: server
server: install
	@echo "Generating Go server stubs..."
	@mkdir -p $(OUTPUT_DIR)
	oapi-codegen -generate types,server -o $(OUTPUT_DIR)/$(PACKAGE_NAME)_server.gen.go -package $(PACKAGE_NAME) $(OPENAPI_FILE)
	go fmt ./$(OUTPUT_DIR)

# --- Generate Go types only ---
.PHONY: types
types: install
	@echo "Generating Go types..."
	@mkdir -p $(OUTPUT_DIR)
	oapi-codegen -generate types -o $(OUTPUT_DIR)/$(PACKAGE_NAME)_types.gen.go -package $(PACKAGE_NAME) $(OPENAPI_FILE)
	go fmt ./$(OUTPUT_DIR)

# --- Generate everything (client + server + types) ---
.PHONY: allgen
allgen: client server types
	@echo "✅ All code generated successfully."

# --- Clean up ---
.PHONY: clean
clean:
	rm -rf $(OUTPUT_DIR)
	@echo "🧹 Cleaned generated files."
