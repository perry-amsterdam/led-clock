#pragma once
#include <ctime>

enum class ThemeStatus : uint8_t
{
	WifiNotConnected,
	PortalActive,
	TimeReady,
};

struct Theme
{
	const char* id;
	const char* name;
	void (*begin)();
	void (*update)(const tm& now, time_t epoch);
	void (*showStatus)(ThemeStatus status);
	uint16_t (*frameDelayMs)();
};
