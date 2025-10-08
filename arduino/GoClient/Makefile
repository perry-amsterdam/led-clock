# Makefile for GoClient
# - Genereert Go code uit ../docs/openapi.json met oapi-codegen
# - Bouwt een kleine CLI (cmd/ledclockctl)

# ===== Config =====
OPENAPI_FILE := ../docs/openapi.json
OUTPUT_DIR   := gen
PACKAGE_NAME := ledclock

BIN      := ledclockctl
CMD_DIR  := ./cmd/ledclockctl

OAPI_CODEGEN := $(shell command -v oapi-codegen 2>/dev/null)

# ===== Default =====
.PHONY: all
all: help

.PHONY: help
help:
	@echo "Targets:"
	@echo "  install        - Installeer oapi-codegen (indien nodig)"
	@echo "  verify         - Check of $(OPENAPI_FILE) bestaat"
	@echo "  client         - Genereer Go client (types+client)"
	@echo "  server         - Genereer Go server stubs (types+server)"
	@echo "  types          - Genereer alleen Go types"
	@echo "  allgen         - Genereer client, server en types"
	@echo "  fmt            - go fmt over ./$(OUTPUT_DIR)"
	@echo "  cli            - Build $(BIN) (hangt af van 'client')"
	@echo "  run            - Run $(BIN) met default flags"
	@echo "  install-cli    - go install van de CLI"
	@echo "  clean          - Verwijder gen/ en $(BIN)"
	@echo "  all            - (deze help)"

# ===== Tooling =====
.PHONY: install
install:
ifeq ($(OAPI_CODEGEN),)
	@echo "Installing oapi-codegen..."
	GO111MODULE=on go install github.com/deepmap/oapi-codegen/v2/cmd/oapi-codegen@latest
else
	@echo "oapi-codegen already installed: $(OAPI_CODEGEN)"
endif

.PHONY: verify
verify:
	@test -f "$(OPENAPI_FILE)" || (echo "❌ Missing $(OPENAPI_FILE)"; exit 1)
	@echo "✅ Found $(OPENAPI_FILE)"

# ===== Code generatie =====
.PHONY: client
client: install verify
	@echo "Generating Go client..."
	@mkdir -p $(OUTPUT_DIR)
	oapi-codegen -generate types,client -o $(OUTPUT_DIR)/$(PACKAGE_NAME)_client.gen.go -package $(PACKAGE_NAME) $(OPENAPI_FILE)
	$(MAKE) fmt

.PHONY: server
server: install verify
	@echo "Generating Go server stubs..."
	@mkdir -p $(OUTPUT_DIR)
	oapi-codegen -generate types,server -o $(OUTPUT_DIR)/$(PACKAGE_NAME)_server.gen.go -package $(PACKAGE_NAME) $(OPENAPI_FILE)
	$(MAKE) fmt

.PHONY: types
types: install verify
	@echo "Generating Go types..."
	@mkdir -p $(OUTPUT_DIR)
	oapi-codegen -generate types -o $(OUTPUT_DIR)/$(PACKAGE_NAME)_types.gen.go -package $(PACKAGE_NAME) $(OPENAPI_FILE)
	$(MAKE) fmt

.PHONY: allgen
allgen: client server types
	@echo "✅ All code generated in ./$(OUTPUT_DIR)"

.PHONY: fmt
fmt:
	@if [ -d "$(OUTPUT_DIR)" ]; then go fmt ./$(OUTPUT_DIR); fi

# ===== CLI =====
.PHONY: cli
cli: client
	@echo "Building $(BIN)..."
	go build -o $(BIN) $(CMD_DIR)
	@echo "✅ Built ./$(BIN)"

.PHONY: run
run: cli
	@echo "Running $(BIN)..."
	./$(BIN) -base http://ledclock.local

.PHONY: install-cli
install-cli: cli
	@echo "Installing $(BIN) to $$GOBIN (or GOPATH/bin)..."
	go install $(CMD_DIR)

# ===== Opschonen =====
.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -rf $(OUTPUT_DIR) $(BIN)
	@echo "🧹 Done."
