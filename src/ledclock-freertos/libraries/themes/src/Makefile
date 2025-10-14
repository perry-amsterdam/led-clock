# Makefile for libraries/themes/src
# Gebruik:
#   make list
#       Toon beschikbare themes (.cpp + .h met prefix 'theme_')
#
#   make clone FROM=theme_default NEW=theme_kerst
#       Clone een bestaand theme (zowel .cpp als .h)
#
#   make help
#       Toon deze hulptekst
#
# Werkt op Linux en macOS.

SHELL := /bin/bash
THEMES_DIR := $(CURDIR)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  SED_INPLACE = sed -i ''
else
  SED_INPLACE = sed -i
endif

.PHONY: help list clone

help:
	@echo "Gebruik:"
	@echo "  make list"
	@echo "      Toon beschikbare themes (.cpp/.h bestanden met prefix 'theme_')."
	@echo ""
	@echo "  make clone FROM=<bestaand> NEW=<nieuw>"
	@echo "      Clone een bestaand theme (.cpp en .h)."
	@echo "      Voorbeeld: make clone FROM=theme_default NEW=theme_kerst"
	@echo ""
	@echo "Deze Makefile hoort te staan in: $(THEMES_DIR)"

list:
	@echo "Beschikbare themes:"
	@ls | grep 'theme_' | grep '\.cpp' | grep -v manager | sed -e "s/.cpp//g" || echo "(geen themes gevonden)"

# Usage: make clone FROM=theme_default NEW=theme_kerst
clone:
	@if [ -z "$(FROM)" ] || [ -z "$(NEW)" ]; then \
		echo "Gebruik: make clone FROM=<bestaand> NEW=<nieuw>"; \
		exit 1; \
	fi
	@if [ ! -f "$(FROM).cpp" ] || [ ! -f "$(FROM).h" ]; then \
		echo "Bron-theme '$(FROM)' bestaat niet (cpp en/of h ontbreekt)."; \
		exit 1; \
	fi
	@if [ -e "$(NEW).cpp" ] || [ -e "$(NEW).h" ]; then \
		echo "Doel-theme '$(NEW)' bestaat al. Kies een andere naam."; \
		exit 1; \
	fi
	@echo "Clonen van '$(FROM)' naar '$(NEW)' ..."
	@cp "$(FROM).cpp" "$(NEW).cpp"
	@cp "$(FROM).h" "$(NEW).h"
	@echo "Vervang naam '$(FROM)' → '$(NEW)' in beide bestanden ..."
	@$(SED_INPLACE) "s/$(FROM)/$(NEW)/g" "$(NEW).cpp"
	@$(SED_INPLACE) "s/$(FROM)/$(NEW)/g" "$(NEW).h"
	@echo "Klaar: nieuw theme-bestand: $(NEW).cpp en $(NEW).h"
	@echo "Pas REGISTER_* macro en Theme id/name waar mogelijk aan ..."
	@$(SED_INPLACE) "s/REGISTER_THEME(.*)/REGISTER_THEME($(NEW));/g" "$(NEW).cpp"
	@$(SED_INPLACE) "s/REGISTER_DEFAULT_THEME(.*)/REGISTER_DEFAULT_THEME($(NEW));/g" "$(NEW).cpp"
	@$(SED_INPLACE) "s/id\s*=\s*\"[^\"]*\"/id=\"$(subst theme_,,$(NEW))\"/g" "$(NEW).cpp"
	@$(SED_INPLACE) "s/name\s*=\s*\"[^\"]*\"/name=\"$(subst theme_,,$(NEW))\"/g" "$(NEW).cpp"
