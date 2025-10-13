#pragma once
#include <ctime>

struct Theme
{
	const char* name;
	void (*begin)();
	void (*update)(const tm& now, time_t epoch);
	void (*showStartupPattern)(uint8_t r, uint8_t g, uint8_t b);
};
