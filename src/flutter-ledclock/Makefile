# Flutter Makefile voor flutter-ledclock

APP_NAME = flutter-ledclock

.PHONY: help create run build clean pubget

help:
	@echo ""
	@echo "Flutter Makefile commando's voor $(APP_NAME):"
	@echo "  make create   - Maak platformbestanden aan (flutter create .)"
	@echo "  make pubget   - Haal dependencies op (flutter pub get)"
	@echo "  make run      - Start de app op standaard device (flutter run)"
	@echo "  make build    - Build een release APK (flutter build apk)"
	@echo "  make install  - Install the built flutter-apk using adb"
	@echo "  make clean    - Verwijder build output en .dart_tool/"
	@echo "  make format   - Format all Dart files in lib/ using flutter format (line length 128)"
	@echo ""

create:
	@echo "🏗️  Genereren van platformbestanden..."
	flutter create .

run:
	@echo "🚀  Starten van $(APP_NAME)..."
	flutter run

build:
	@echo "📦  Bouwen van release APK..."
	flutter build apk --release

pubget:
	@echo "📚  Ophalen van Flutter dependencies..."
	flutter pub get

clean:
	@echo "🧹  Opruimen van buildbestanden..."
	flutter clean
	rm -rf .dart_tool

install:
	find -type f -name "*.apk" | grep flutter-apk | xargs adb install


format:
	dart format lib --line-length 128
