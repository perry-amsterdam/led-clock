import 'package:flutter/material.dart';

const ikeaBlue = Color(0xFF0058AB);
const ikeaYellow = Color(0xFFFBD914);

final ColorScheme _lightScheme = const ColorScheme(
  brightness: Brightness.light,
  primary: ikeaBlue,
  onPrimary: Colors.white,
  primaryContainer: Color(0xFFCCE1F6),
  onPrimaryContainer: Color(0xFF001C36),
  secondary: ikeaYellow,
  onSecondary: Colors.black,
  secondaryContainer: Color(0xFFFFF3B0),
  onSecondaryContainer: Color(0xFF231B00),
  tertiary: Color(0xFF4A6FA5),
  onTertiary: Colors.white,
  error: Color(0xFFB3261E),
  onError: Colors.white,
  background: Colors.white,
  onBackground: Color(0xFF111111),
  surface: Colors.white,
  onSurface: Color(0xFF111111),
  surfaceVariant: Color(0xFFE8E8ED),
  onSurfaceVariant: Color(0xFF44474E),
  outline: Color(0xFF74777F),
  shadow: Colors.black,
  inverseSurface: Color(0xFF1F1F1F),
  onInverseSurface: Colors.white,
  inversePrimary: Color(0xFFA4C8F2),
);

final ColorScheme _darkScheme = const ColorScheme(
  brightness: Brightness.dark,
  primary: Color(0xFF7FB5E8),
  onPrimary: Color(0xFF001527),
  primaryContainer: Color(0xFF004275),
  onPrimaryContainer: Colors.white,
  secondary: Color(0xFFFFE067),
  onSecondary: Color(0xFF221B00),
  secondaryContainer: Color(0xFF574800),
  onSecondaryContainer: Colors.white,
  tertiary: Color(0xFF9EC2F2),
  onTertiary: Color(0xFF09223C),
  error: Color(0xFFF2B8B5),
  onError: Color(0xFF601410),
  background: Color(0xFF121212),
  onBackground: Colors.white,
  surface: Color(0xFF121212),
  onSurface: Colors.white,
  surfaceVariant: Color(0xFF2C2F36),
  onSurfaceVariant: Color(0xFFC4C6CF),
  outline: Color(0xFF8E9199),
  shadow: Colors.black,
  inverseSurface: Colors.white,
  onInverseSurface: Color(0xFF1F1F1F),
  inversePrimary: Color(0xFF2F6FB0),
);

ThemeData ikeaLightTheme() {
  final scheme = _lightScheme;
  return ThemeData(
    useMaterial3: true,
    colorScheme: scheme,
    appBarTheme: AppBarTheme(
      backgroundColor: scheme.primary,
      foregroundColor: scheme.onPrimary,
      centerTitle: true,
      elevation: 0,
    ),
    elevatedButtonTheme: ElevatedButtonThemeData(
      style: ElevatedButton.styleFrom(
        backgroundColor: scheme.primary,
        foregroundColor: scheme.onPrimary,
        padding: const EdgeInsets.symmetric(horizontal: 20, vertical: 14),
        shape: const StadiumBorder(),
      ),
    ),
    textButtonTheme: TextButtonThemeData(style: TextButton.styleFrom(foregroundColor: scheme.primary)),
    floatingActionButtonTheme: FloatingActionButtonThemeData(
      backgroundColor: scheme.secondary,
      foregroundColor: scheme.onSecondary,
    ),
    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: scheme.surfaceVariant,
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(12),
        borderSide: BorderSide(color: scheme.outline),
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(12),
        borderSide: BorderSide(color: scheme.primary, width: 2),
      ),
    ),
    cardTheme: CardThemeData(
      elevation: 0,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
      margin: const EdgeInsets.all(12),
    ),
    snackBarTheme: SnackBarThemeData(
      backgroundColor: scheme.primary,
      contentTextStyle: TextStyle(color: scheme.onPrimary),
      behavior: SnackBarBehavior.floating,
    ),
    dividerTheme: DividerThemeData(thickness: 1),
    chipTheme: ChipThemeData(
      backgroundColor: scheme.secondaryContainer,
      labelStyle: TextStyle(color: scheme.onSecondaryContainer),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
    ),
  );
}

ThemeData ikeaDarkTheme() {
  final scheme = _darkScheme;
  final base = ikeaLightTheme();
  return base.copyWith(
    colorScheme: scheme,
    appBarTheme: AppBarTheme(backgroundColor: scheme.surface, foregroundColor: scheme.onSurface, centerTitle: true),
    snackBarTheme: base.snackBarTheme.copyWith(
      backgroundColor: scheme.primary,
      contentTextStyle: TextStyle(color: scheme.onPrimary),
    ),
  );
}
