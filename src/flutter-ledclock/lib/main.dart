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
