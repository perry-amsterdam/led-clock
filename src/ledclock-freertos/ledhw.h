#pragma once
#include <stdint.h>

void ledhwInitOnce();
void ledhwSetGlobalBrightness(uint8_t b);
void ledhwClearAll();
void ledhwShow();

uint16_t ring60Index(int raw);
uint16_t ring24Index(int raw);

void ledhwSet60(uint16_t i, uint8_t r, uint8_t g, uint8_t b);
void ledhwSet24(uint16_t i, uint8_t r, uint8_t g, uint8_t b);

void ledhwAdd60(uint16_t i, uint8_t r, uint8_t g, uint8_t b);
void ledhwAdd24(uint16_t i, uint8_t r, uint8_t g, uint8_t b);
