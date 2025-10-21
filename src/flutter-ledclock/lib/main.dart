import 'package:flutter/material.dart';
import 'led_clock_api.dart';
import 'led_clock_widgets.dart';

void main() => runApp(const MyApp());

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: LedClockControlPanel(host: 'ledclock.local'), // of "192.168.1.50:80"
      ),
    );
  }
}
