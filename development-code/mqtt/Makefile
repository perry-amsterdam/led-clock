# De naam van de uitvoerbare applicatie
APP = mqtt_client

# De Go-bestanden die gecompileerd moeten worden
SRC = main.go

# Standaard build target
build:
	@echo "Building the application..."
	go build -o $(APP) $(SRC)

# Run de applicatie
run: build
	@echo "Running the application..."
	./$(APP)

# Dependencies installeren
deps:
	@echo "Installing dependencies..."
	go mod tidy

# Clean de opgebouwde bestanden
clean:
	@echo "Cleaning up..."
	rm -f $(APP)

# Alles opnieuw uitvoeren (clean + build + run)
rebuild: clean build run

# Help commando
help:
	@echo "Makefile commando's:"
	@echo "  build    - Bouw de Go-applicatie"
	@echo "  run      - Bouw en voer de applicatie uit"
	@echo "  deps     - Installeer benodigde dependencies"
	@echo "  clean    - Verwijder gegenereerde bestanden"
	@echo "  rebuild  - Rebuild de applicatie (clean + build + run)"

