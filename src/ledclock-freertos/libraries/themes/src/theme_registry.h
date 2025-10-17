#pragma once
#include <stddef.h>
#include <string.h>
#include "theme.h"

// ============================================================================
// ThemeRegistry - Lightweight static registry for Theme objects
// ============================================================================
// This class provides a simple, static (non-dynamic) registry system to manage
// a limited number of Theme objects. It avoids dynamic memory allocation (heap)
// by using fixed-size static arrays and counters. This is ideal for use in
// embedded systems such as the ESP32, where memory management is critical.
// ============================================================================
struct ThemeRegistry
{
	static constexpr size_t kMaxThemes = 8;
	static inline const Theme* s_items[kMaxThemes] = { nullptr };
	static inline size_t       s_count             = 0;
	static inline const Theme* s_default           = nullptr;

	// ------------------------------------------------------------------------
	// add()
	// Adds a theme pointer to the registry, if there is space left.
	// Does not allocate memory or copy data  only stores the pointer.
	// ------------------------------------------------------------------------
	static void add(const Theme* t)
	{
		if (!t) return;
		if (s_count >= kMaxThemes) return;
		for (size_t i = 0; i < s_count; ++i)
		{
			if (s_items[i] == t) return;
			if (s_items[i] && s_items[i]->id && t->id && strcmp(s_items[i]->id, t->id) == 0) return;
		}
		s_items[s_count++] = t;
	}

	// ------------------------------------------------------------------------
	// setDefault() / getDefault()
	// Allows setting and retrieving a default theme.
	// If no default theme is explicitly set, getDefault() returns the first
	// registered theme (if available).
	// ------------------------------------------------------------------------
	static void setDefault(const Theme* t)
	{
		s_default = t;
	}
	static const Theme* getDefault()
	{
		return s_default ? s_default : (s_count ? s_items[0] : nullptr);
	}

	// ------------------------------------------------------------------------
	// findById()
	// Finds a theme by its unique identifier (string ID).
	// Returns a pointer to the matching Theme, or nullptr if not found.
	// ------------------------------------------------------------------------
	static const Theme* findById(const char* id)
	{
		if (!id || !id[0]) return nullptr;
		for (size_t i = 0; i < s_count; ++i)
		{
			const Theme* t = s_items[i];
			if (t && t->id && strcmp(t->id, id) == 0) return t;
		}
		return nullptr;
	}

	// ------------------------------------------------------------------------
	// findByName()
	// Finds a theme by its display name (human-readable name).
	// Returns a pointer to the matching Theme, or nullptr if not found.
	// ------------------------------------------------------------------------
	static const Theme* findByName(const char* name)
	{
		if (!name || !name[0]) return nullptr;
		for (size_t i = 0; i < s_count; ++i)
		{
			const Theme* t = s_items[i];
			if (t && t->name && strcmp(t->name, name) == 0) return t;
		}
		return nullptr;
	}

	static const Theme* const* items()
	{
		return s_items;
	}

	static size_t size()
	{
		return s_count;
	}

	static size_t count()
	{
		return s_count;
	}

	static const Theme* def()
	{
		return s_default;
	}
};

// ---------------------------------------------------------------------------
// Auto-registratie van thema's
// ---------------------------------------------------------------------------
// Gebruik REGISTER_THEME(myTheme) om een thema automatisch te registreren
// bij opstarten van het programma. De macro maakt een verborgen struct met
// een constructor die ThemeRegistry::add() aanroept. Zo worden alle themas
// automatisch toegevoegd zonder handmatig beheer.
// ---------------------------------------------------------------------------

#define REGISTER_THEME(theme_sym) \
	namespace \
	{ \
		struct _ThemeReg_##theme_sym \
		{ \
			_ThemeReg_##theme_sym() { ThemeRegistry::add(&(theme_sym)); } \
		} _ThemeRegInst_##theme_sym; \
	}

// ---------------------------------------------------------------------------
// Auto-registratie van het standaardthema
// ---------------------------------------------------------------------------
// Gebruik REGISTER_DEFAULT_THEME(myTheme) om een thema automatisch te
// registreren n als standaardthema in te stellen bij opstarten.
// De macro maakt een verborgen struct met een constructor die zowel
// ThemeRegistry::add() als ThemeRegistry::setDefault() aanroept.
// ---------------------------------------------------------------------------

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
