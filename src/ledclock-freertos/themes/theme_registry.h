#pragma once
#include <stddef.h>
#include "themes/theme.h"

// Lichtgewicht registry zonder heap
struct ThemeRegistry
{
	static constexpr size_t kMax = 16;

	static void add(const Theme* t)
	{
		if (s_count < kMax) s_items[s_count++] = t;
	}

	static size_t size() { return s_count; }
	static const Theme* const* items() { return s_items; }

	static void setDefault(const Theme* t) { s_default = t; }
	static const Theme* getDefault()
	{
		return s_default ? s_default : (s_count ? s_items[0] : nullptr);
	}

	private:
		static const Theme* s_items[kMax];
		static size_t s_count;
		static const Theme* s_default;
};

// Definities (nmalig in precies n .cpp includen of hier laten als header-only)
inline const Theme* ThemeRegistry::s_items[ThemeRegistry::kMax] = {};
inline size_t       ThemeRegistry::s_count = 0;
inline const Theme* ThemeRegistry::s_default = nullptr;

// Macro om een thema automatisch te registreren
#define REGISTER_THEME(theme_sym) \
	namespace \
	{ \
		struct _ThemeReg_##theme_sym \
		{ \
			_ThemeReg_##theme_sym() { ThemeRegistry::add(&(theme_sym)); } \
		} _ThemeRegInst_##theme_sym; \
	}

// Zelfde, maar ook als default markeren
#define REGISTER_DEFAULT_THEME(theme_sym) \
	namespace \
	{ \
		struct _ThemeDef_##theme_sym \
		{ \
			_ThemeDef_##theme_sym() \
			{ \
				ThemeRegistry::add(&(theme_sym)); \
				ThemeRegistry::setDefault(&(theme_sym)); \
			} \
		} _ThemeDefInst_##theme_sym; \
	}
