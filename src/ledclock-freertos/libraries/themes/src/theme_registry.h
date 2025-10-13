#pragma once
#include <stddef.h>
#include <string.h>
#include "theme.h"

// Lichtgewicht registry zonder heap
struct ThemeRegistry
{
    static constexpr size_t kMax = 16;

    static void add(const Theme* t)
    {
        if (s_count < kMax) s_items[s_count++] = t;
    }

    static void setDefault(const Theme* t) { s_default = t; }
    static const Theme* getDefault()       { return s_default ? s_default : (s_count ? s_items[0] : nullptr); }

    static size_t size()                   { return s_count; }
    static const Theme* const* items()     { return s_items; }

    // Nieuw: lookup helpers
    static const Theme* findById(const char* id)
    {
        if (!id) return nullptr;
        for (size_t i=0;i<s_count;i++) if (s_items[i]->id && strcmp(s_items[i]->id, id)==0) return s_items[i];
        return nullptr;
    }

    static const Theme* findByName(const char* name)
    {
        if (!name) return nullptr;
        for (size_t i=0;i<s_count;i++) if (s_items[i]->name && strcmp(s_items[i]->name, name)==0) return s_items[i];
        return nullptr;
    }

private:
    static inline const Theme* s_items[kMax] = {};
    static inline size_t       s_count       = 0;
    static inline const Theme* s_default     = nullptr;
};

// Auto-registratie (zoals je al had)
#define REGISTER_THEME(theme_sym) \
    namespace { struct _ThemeDef_##theme_sym { _ThemeDef_##theme_sym(){ ThemeRegistry::add(&(theme_sym)); } } _ThemeDefInst_##theme_sym; }

#define REGISTER_DEFAULT_THEME(theme_sym) \
    namespace { struct _ThemeDef_##theme_sym { _ThemeDef_##theme_sym(){ ThemeRegistry::add(&(theme_sym)); ThemeRegistry::setDefault(&(theme_sym)); } } _ThemeDefInst_##theme_sym; }

