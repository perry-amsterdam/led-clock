cmake_minimum_required(VERSION 3.20.0)

# Zoek Zephyr in de workspace
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(led_clock)

# Voeg jouw source bestanden toe
target_sources(app PRIVATE src/main.c)
