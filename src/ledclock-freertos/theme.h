#pragma once
#include <ctime>

struct Theme
{
	const char* name;
	void (*begin)();
	void (*update)(const tm& now, time_t epoch);
};
