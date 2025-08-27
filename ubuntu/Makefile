# Makefile for building ip_tz_ntp on Ubuntu
# Usage:
#   make            # build
#   make run        # build + run
#   make debug      # build with -g -O0
#   make clean      # remove binary

CC      ?= gcc
TARGET  ?= ip_tz_ntp
SRC     ?= ip_tz_ntp.c
PKG_CONFIG ?= pkg-config

# Base warnings/flags
CFLAGS  ?= -O2 -Wall -Wextra -Wpedantic -std=c11
LDFLAGS ?=

# Try to discover cflags/libs via pkg-config, fall back if unavailable
CURL_CFLAGS  := $(shell $(PKG_CONFIG) --cflags libcurl 2>/dev/null)
CURL_LIBS    := $(shell $(PKG_CONFIG) --libs   libcurl 2>/dev/null)
CJSON_CFLAGS := $(shell $(PKG_CONFIG) --cflags cjson   2>/dev/null)
CJSON_LIBS   := $(shell $(PKG_CONFIG) --libs   cjson   2>/dev/null)

ifeq ($(strip $(CURL_LIBS)),)
  # Fallback if pkg-config for libcurl is not present
  CURL_LIBS := -lcurl
endif
ifeq ($(strip $(CJSON_LIBS)),)
  # Fallback if pkg-config for cJSON is not present
  CJSON_LIBS := -lcjson
endif

ALL_CFLAGS := $(CFLAGS) $(CURL_CFLAGS) $(CJSON_CFLAGS)
ALL_LIBS   := $(LDFLAGS) $(CURL_LIBS) $(CJSON_LIBS)

.PHONY: all run clean debug

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(ALL_CFLAGS) -o $@ $< $(ALL_LIBS)

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -g -O0
debug: clean all

clean:
	rm -f $(TARGET)
