import 'dart:developer' as dev;
import 'package:flutter/material.dart';
import 'led_clock_api.dart';
import 'led_clock_widgets.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await HostConfig.instance.init();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        body: LedClockControlPanel(host: HostConfig.instance.host)
      ),
    );
  }
}


