import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'led_clock_api.dart';
import 'led_clock_widgets.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        useMaterial3: true,
        colorScheme: ColorScheme.fromSeed(
          seedColor: Color(0xFF0058A3),
          primary: Color(0xFF0058A3),
          secondary: Color(0xFFFFCC00),
          brightness: Brightness.light,
        ),
        scaffoldBackgroundColor: const Color(0xFFF7F7F7),
        appBarTheme: const AppBarTheme(centerTitle: false, elevation: 0),
        textTheme: const TextTheme(
          headlineSmall: TextStyle(fontWeight: FontWeight.w700, letterSpacing: -0.2),
          titleMedium: TextStyle(fontWeight: FontWeight.w600),
        ),
        inputDecorationTheme: const InputDecorationTheme(
          border: OutlineInputBorder(borderRadius: BorderRadius.all(Radius.circular(12))),
          focusedBorder: OutlineInputBorder(borderSide: BorderSide(width: 2)),
          isDense: true,
          contentPadding: EdgeInsets.symmetric(horizontal: 12, vertical: 10),
        ),
        cardTheme: const CardThemeData(
          elevation: 0.5,
          margin: EdgeInsets.symmetric(vertical: 8),
          shape: RoundedRectangleBorder(borderRadius: BorderRadius.all(Radius.circular(16))),
        ),
        elevatedButtonTheme: ElevatedButtonThemeData(
          style: ButtonStyle(
            shape: MaterialStatePropertyAll(RoundedRectangleBorder(borderRadius: BorderRadius.circular(12))),
            padding: const MaterialStatePropertyAll(EdgeInsets.symmetric(horizontal: 14, vertical: 12)),
          ),
        ),
        filledButtonTheme: FilledButtonThemeData(
          style: ButtonStyle(
            shape: MaterialStatePropertyAll(RoundedRectangleBorder(borderRadius: BorderRadius.circular(12))),
            padding: const MaterialStatePropertyAll(EdgeInsets.symmetric(horizontal: 14, vertical: 12)),
            backgroundColor: const MaterialStatePropertyAll(Color(0xFF0058A3)),
            foregroundColor: const MaterialStatePropertyAll(Colors.white),
          ),
        ),
      ),
      home: const HostLoader(),
    );
  }
}

class HostLoader extends StatefulWidget {
  const HostLoader({super.key});
  @override
  State<HostLoader> createState() => _HostLoaderState();
}

class _HostLoaderState extends State<HostLoader> {
  String _host = 'ledclock.local';

  @override
  void initState() {
    super.initState();
    _loadHost();
  }

  Future<void> _loadHost() async {
    final prefs = await SharedPreferences.getInstance();
    setState(() {
      _host = prefs.getString('ledclock_host') ?? 'ledclock.local';
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: LedClockControlPanel(initialHost: _host),
    );
  }
}
