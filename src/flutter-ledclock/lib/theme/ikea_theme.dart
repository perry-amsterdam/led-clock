// lib/theme/ikea_theme.dart
import 'package:flutter/material.dart';

/// IKEA-inspired design tokens
/// Note: IKEA uses proprietary fonts; we approximate with system-friendly sans-serifs.
class IkeaTokens {
  // Brand
  static const Color ikeaBlue = Color(0xFF0058A3);
  static const Color ikeaYellow = Color(0xFFFFDA1A);

  // Neutrals
  static const Color neutral0 = Color(0xFFFFFFFF);
  static const Color neutral10 = Color(0xFFF6F6F6);
  static const Color neutral20 = Color(0xFFEAEAEA);
  static const Color neutral40 = Color(0xFFBDBDBD);
  static const Color neutral60 = Color(0xFF7A7A7A);
  static const Color neutral80 = Color(0xFF3C3C3C);
  static const Color neutral90 = Color(0xFF1E1E1E);

  // Radii / sizes
  static const double radius = 12;
  static const double buttonHeight = 56; // large, comfy
  static const EdgeInsets contentPadding = EdgeInsets.symmetric(horizontal: 20, vertical: 14);
}

/// A helper that creates vivid, accessible overlay colors for hover/press/focus.
MaterialStateProperty<Color?> overlayFrom(Color base, {double hover = .08, double pressed = .14, double focus = .12}) {
  return MaterialStateProperty.resolveWith((states) {
    if (states.contains(MaterialState.pressed)) {
      return base.withOpacity(pressed);
    }
    if (states.contains(MaterialState.hovered)) {
      return base.withOpacity(hover);
    }
    if (states.contains(MaterialState.focused)) {
      return base.withOpacity(focus);
    }
    return null;
  });
}

ButtonStyle largeButtonStyle(ColorScheme scheme) {
  final shape = RoundedRectangleBorder(
    borderRadius: BorderRadius.circular(IkeaTokens.radius),
  );
  return ButtonStyle(
    minimumSize: const MaterialStatePropertyAll(Size.fromHeight(IkeaTokens.buttonHeight)),
    padding: const MaterialStatePropertyAll(IkeaTokens.contentPadding),
    shape: MaterialStatePropertyAll(shape),
    elevation: const MaterialStatePropertyAll(0),
    overlayColor: overlayFrom(scheme.onPrimary),
    animationDuration: const Duration(milliseconds: 90),
    splashFactory: InkSparkle.splashFactory,
  );
}

ThemeData ikeaLightTheme() {
  final seed = IkeaTokens.ikeaBlue;
  final scheme = ColorScheme.fromSeed(
    seedColor: seed,
    brightness: Brightness.light,
    primary: IkeaTokens.ikeaBlue,
    secondary: IkeaTokens.ikeaYellow,
  );

  final textTheme = Typography.blackMountainView.apply(
    bodyColor: IkeaTokens.neutral90,
    displayColor: IkeaTokens.neutral90,
  ).copyWith(
    titleLarge: const TextStyle(fontWeight: FontWeight.w700),
    labelLarge: const TextStyle(fontWeight: FontWeight.w700, letterSpacing: .2),
  );

  return ThemeData(
    useMaterial3: true,
    colorScheme: scheme,
    scaffoldBackgroundColor: IkeaTokens.neutral0,
    textTheme: textTheme,
    // AppBar
    appBarTheme: AppBarTheme(
      backgroundColor: scheme.surface,
      foregroundColor: scheme.onSurface,
      elevation: 0,
      scrolledUnderElevation: 0,
      centerTitle: false,
      titleTextStyle: textTheme.titleLarge,
    ),
    // Cards
    cardTheme: CardTheme(
      color: scheme.surface,
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(IkeaTokens.radius),
        side: BorderSide(color: IkeaTokens.neutral20),
      ),
      margin: const EdgeInsets.all(12),
    ),
    // Buttons
    filledButtonTheme: FilledButtonThemeData(
      style: largeButtonStyle(scheme).copyWith(
        backgroundColor: MaterialStateProperty.resolveWith((states) {
          if (states.contains(MaterialState.disabled)) return scheme.primary.withOpacity(.35);
          return scheme.primary;
        }),
        foregroundColor: const MaterialStatePropertyAll(Colors.white),
      ),
    ),
    elevatedButtonTheme: ElevatedButtonThemeData(
      style: largeButtonStyle(scheme).copyWith(
        backgroundColor: const MaterialStatePropertyAll(Colors.white),
        foregroundColor: MaterialStatePropertyAll(scheme.primary),
        side: MaterialStatePropertyAll(BorderSide(color: scheme.primary, width: 2)),
        overlayColor: overlayFrom(scheme.primary),
      ),
    ),
    outlinedButtonTheme: OutlinedButtonThemeData(
      style: largeButtonStyle(scheme).copyWith(
        side: MaterialStatePropertyAll(BorderSide(color: IkeaTokens.neutral40)),
        foregroundColor: MaterialStatePropertyAll(scheme.primary),
        overlayColor: overlayFrom(scheme.primary),
      ),
    ),
    textButtonTheme: TextButtonThemeData(
      style: ButtonStyle(
        padding: const MaterialStatePropertyAll(EdgeInsets.symmetric(horizontal: 8, vertical: 6)),
        foregroundColor: MaterialStatePropertyAll(scheme.primary),
        overlayColor: overlayFrom(scheme.primary),
        textStyle: const MaterialStatePropertyAll(TextStyle(fontWeight: FontWeight.w700)),
      ),
    ),
    iconButtonTheme: IconButtonThemeData(
      style: ButtonStyle(
        minimumSize: const MaterialStatePropertyAll(Size.square(48)),
        iconSize: const MaterialStatePropertyAll(24),
        shape: MaterialStatePropertyAll(
          RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
        ),
        overlayColor: overlayFrom(scheme.primary),
      ),
    ),
    // Inputs
    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: IkeaTokens.neutral10,
      contentPadding: IkeaTokens.contentPadding,
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(IkeaTokens.radius),
        borderSide: BorderSide.none,
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(IkeaTokens.radius),
        borderSide: BorderSide(color: scheme.primary, width: 2),
      ),
      hoverColor: scheme.primary.withOpacity(.06),
    ),
    // List tiles
    listTileTheme: ListTileThemeData(
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(IkeaTokens.radius)),
      tileColor: Colors.white,
      selectedTileColor: scheme.primary.withOpacity(.08),
      contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
    ),
    // Switches/checkboxes
    switchTheme: SwitchThemeData(
      trackColor: MaterialStateProperty.resolveWith((states) {
        if (states.contains(MaterialState.selected)) return scheme.primary.withOpacity(.4);
        return IkeaTokens.neutral40.withOpacity(.4);
      }),
      thumbColor: MaterialStateProperty.resolveWith((states) {
        if (states.contains(MaterialState.selected)) return scheme.primary;
        return IkeaTokens.neutral60;
      }),
    ),
    dividerTheme: DividerThemeData(
      color: IkeaTokens.neutral20,
      thickness: 1,
      space: 24,
    ),
    splashFactory: InkSparkle.splashFactory,
    visualDensity: VisualDensity.standard,
  );
}

ThemeData ikeaDarkTheme() {
  final seed = IkeaTokens.ikeaBlue;
  final scheme = ColorScheme.fromSeed(
    seedColor: seed,
    brightness: Brightness.dark,
    primary: IkeaTokens.ikeaYellow, // pop on dark
    secondary: IkeaTokens.ikeaBlue,
  );

  final textTheme = Typography.whiteMountainView.apply(
    bodyColor: IkeaTokens.neutral0,
    displayColor: IkeaTokens.neutral0,
  ).copyWith(
    titleLarge: const TextStyle(fontWeight: FontWeight.w700),
    labelLarge: const TextStyle(fontWeight: FontWeight.w700, letterSpacing: .2),
  );

  return ThemeData(
    useMaterial3: true,
    colorScheme: scheme,
    scaffoldBackgroundColor: IkeaTokens.neutral90,
    textTheme: textTheme,
    appBarTheme: AppBarTheme(
      backgroundColor: IkeaTokens.neutral80,
      foregroundColor: Colors.white,
      elevation: 0,
      scrolledUnderElevation: 0,
      centerTitle: false,
      titleTextStyle: textTheme.titleLarge,
    ),
    cardTheme: CardTheme(
      color: IkeaTokens.neutral80,
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(IkeaTokens.radius),
        side: BorderSide(color: Colors.white.withOpacity(.06)),
      ),
      margin: const EdgeInsets.all(12),
    ),
    filledButtonTheme: FilledButtonThemeData(
      style: largeButtonStyle(scheme).copyWith(
        backgroundColor: const MaterialStatePropertyAll(IkeaTokens.ikeaBlue),
        foregroundColor: const MaterialStatePropertyAll(Colors.white),
        overlayColor: overlayFrom(Colors.white),
      ),
    ),
    elevatedButtonTheme: ElevatedButtonThemeData(
      style: largeButtonStyle(scheme).copyWith(
        backgroundColor: const MaterialStatePropertyAll(IkeaTokens.neutral80),
        foregroundColor: const MaterialStatePropertyAll(Colors.white),
        side: MaterialStatePropertyAll(BorderSide(color: Colors.white.withOpacity(.16), width: 2)),
        overlayColor: overlayFrom(Colors.white),
      ),
    ),
    outlinedButtonTheme: OutlinedButtonThemeData(
      style: largeButtonStyle(scheme).copyWith(
        side: MaterialStatePropertyAll(BorderSide(color: Colors.white.withOpacity(.28))),
        foregroundColor: const MaterialStatePropertyAll(Colors.white),
        overlayColor: overlayFrom(Colors.white),
      ),
    ),
    textButtonTheme: TextButtonThemeData(
      style: ButtonStyle(
        padding: const MaterialStatePropertyAll(EdgeInsets.symmetric(horizontal: 8, vertical: 6)),
        foregroundColor: const MaterialStatePropertyAll(Colors.white),
        overlayColor: overlayFrom(Colors.white),
        textStyle: const MaterialStatePropertyAll(TextStyle(fontWeight: FontWeight.w700)),
      ),
    ),
    iconButtonTheme: IconButtonThemeData(
      style: ButtonStyle(
        minimumSize: const MaterialStatePropertyAll(Size.square(48)),
        iconSize: const MaterialStatePropertyAll(24),
        shape: MaterialStatePropertyAll(
          RoundedRectangleBorder(borderRadius: BorderRadius.circular(16)),
        ),
        overlayColor: overlayFrom(Colors.white),
      ),
    ),
    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: IkeaTokens.neutral80,
      contentPadding: IkeaTokens.contentPadding,
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(IkeaTokens.radius),
        borderSide: BorderSide.none,
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(IkeaTokens.radius),
        borderSide: const BorderSide(color: IkeaTokens.ikeaYellow, width: 2),
      ),
      hoverColor: Colors.white.withOpacity(.06),
    ),
    listTileTheme: ListTileThemeData(
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(IkeaTokens.radius)),
      tileColor: IkeaTokens.neutral80,
      selectedTileColor: Colors.white.withOpacity(.08),
      contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
      iconColor: Colors.white,
      textColor: Colors.white,
    ),
    switchTheme: SwitchThemeData(
      trackColor: MaterialStateProperty.resolveWith((states) {
        if (states.contains(MaterialState.selected)) return IkeaTokens.ikeaBlue.withOpacity(.5);
        return Colors.white24;
      }),
      thumbColor: MaterialStateProperty.resolveWith((states) {
        if (states.contains(MaterialState.selected)) return IkeaTokens.ikeaBlue;
        return Colors.white70;
      }),
    ),
    dividerTheme: DividerThemeData(
      color: Colors.white.withOpacity(.08),
      thickness: 1,
      space: 24,
    ),
    splashFactory: InkSparkle.splashFactory,
    visualDensity: VisualDensity.standard,
  );
}

/// Optional: a ready-to-use big CTA button that matches the theme
class IkeaCtaButton extends StatelessWidget {
  final VoidCallback? onPressed;
  final Widget child;
  final IconData? leading;
  const IkeaCtaButton({super.key, required this.onPressed, required this.child, this.leading});

  @override
  Widget build(BuildContext context) {
    final scheme = Theme.of(context).colorScheme;
    return FilledButton(
      style: largeButtonStyle(scheme),
      onPressed: onPressed,
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        mainAxisSize: MainAxisSize.min,
        children: [
          if (leading != null) ...[Icon(leading, size: 22), const SizedBox(width: 10)],
          DefaultTextStyle.merge(
            style: const TextStyle(fontWeight: FontWeight.w700),
            child: child,
          ),
        ],
      ),
    );
  }
}
