// lib/theme/theme_usage_example.dart
// Drop-in example of how to use the IKEA themes.
import 'package:flutter/material.dart';
import 'ikea_theme.dart';

class ThemeUsageExample extends StatelessWidget {
  const ThemeUsageExample({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'LED Clock (IKEA Style)',
      theme: ikeaLightTheme(),
      darkTheme: ikeaDarkTheme(),
      themeMode: ThemeMode.system,
      home: const _DemoScreen(),
    );
  }
}

class _DemoScreen extends StatelessWidget {
  const _DemoScreen();

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('IKEA UI Demo')),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          const Text('Buttons', style: TextStyle(fontSize: 20, fontWeight: FontWeight.w700)),
          const SizedBox(height: 8),
          Wrap(
            spacing: 12, runSpacing: 12,
            children: [
              SizedBox(width: 260, child: IkeaCtaButton(onPressed: () {}, child: const Text('GROTE KNOP'), leading: Icons.play_circle)),
              SizedBox(width: 220, child: FilledButton(onPressed: () {}, child: const Text('FilledButton'))),
              SizedBox(width: 220, child: ElevatedButton(onPressed: () {}, child: const Text('ElevatedButton'))),
              SizedBox(width: 220, child: OutlinedButton(onPressed: () {}, child: const Text('OutlinedButton'))),
              SizedBox(width: 220, child: TextButton(onPressed: () {}, child: const Text('TextButton'))),
              IconButton(onPressed: () {}, icon: const Icon(Icons.lightbulb_outline)),
              IconButton(onPressed: null, icon: const Icon(Icons.lightbulb_outline)),
            ],
          ),
          const SizedBox(height: 24),
          const Text('Inputs', style: TextStyle(fontSize: 20, fontWeight: FontWeight.w700)),
          const SizedBox(height: 8),
          TextField(decoration: const InputDecoration(hintText: 'Zoeken of command...')),
          const SizedBox(height: 24),
          const Text('List tiles', style: TextStyle(fontSize: 20, fontWeight: FontWeight.w700)),
          const SizedBox(height: 8),
          Card(
            child: Column(
              children: const [
                ListTile(leading: Icon(Icons.schedule), title: Text('Tijdzone'), subtitle: Text('Europe/Amsterdam'), trailing: Icon(Icons.chevron_right)),
                Divider(height: 1),
                ListTile(leading: Icon(Icons.brightness_6), title: Text('Helderheid'), subtitle: Text('75%'), trailing: Switch(value: true, onChanged: null)),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
