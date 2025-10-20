
// main.dart
import 'package:flutter/material.dart';
import 'led_clock_widgets.dart';

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    // IKEA brand colors
    const ikeaBlue = Color(0xFF0058A3);
    const ikeaYellow = Color(0xFFFFCC00);

    final scheme = ColorScheme.fromSeed(seedColor: ikeaBlue).copyWith(
      primary: ikeaBlue,
      secondary: ikeaYellow,
      primaryContainer: ikeaBlue,
      onPrimary: Colors.white,
      onSecondary: Colors.black,
    );

    return MaterialApp(
      title: 'LED Clock Control',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        colorScheme: scheme,
        useMaterial3: true,
        appBarTheme: const AppBarTheme(backgroundColor: Color(0xFF0058A3), foregroundColor: Colors.white),
        elevatedButtonTheme: ElevatedButtonThemeData(
          style: ButtonStyle(
            backgroundColor: WidgetStatePropertyAll(Color(0xFF0058A3)),
            foregroundColor: WidgetStatePropertyAll(Colors.white),
            shape: WidgetStatePropertyAll(RoundedRectangleBorder(borderRadius: BorderRadius.all(Radius.circular(12)))),
          ),
        ),
        sliderTheme: const SliderThemeData(
          activeTrackColor: Color(0xFF0058A3),
          thumbColor: Color(0xFFFFCC00),
          inactiveTrackColor: Color(0xFF0058A3),
        ),
        switchTheme: SwitchThemeData(
          thumbColor: WidgetStateProperty.resolveWith((states) => states.contains(WidgetState.selected) ? const Color(0xFFFFCC00) : Colors.white),
          trackColor: WidgetStateProperty.resolveWith((states) => states.contains(WidgetState.selected) ? const Color(0xFF0058A3) : Colors.black12),
        ),
      ),
      home: LedClockControlPanel(),
    );
  }
}
