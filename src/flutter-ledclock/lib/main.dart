
// main.dart
import 'package:flutter/material.dart';
import 'led_clock_widgets.dart';
import 'theme/ikea_theme.dart';

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

    return MaterialApp(title: 'LED Clock Control', debugShowCheckedModeBanner: false, theme: ikeaLightTheme(), darkTheme: ikeaDarkTheme(), themeMode: ThemeMode.system, home: LedClockControlPanel(),);
  }
}
