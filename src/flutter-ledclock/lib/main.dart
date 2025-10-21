import 'theme/ikea_theme.dart';
import 'package:flutter/material.dart';
import 'host_config.dart';
import 'led_clock_api.dart';
import 'led_clock_widgets.dart'; // of waar LedClockControlPanel staat

void main() {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
        debugShowCheckedModeBanner: false,
        theme: ikeaLightTheme(),
        darkTheme: ikeaDarkTheme(),
        themeMode: ThemeMode.system,
        home: const HomeScreen(),
      );
  }
}

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});
  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  late Future<void> _init;

  @override
  void initState() {
    super.initState();
    _init = HostConfig.instance.init();
  }

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
      future: _init,
      builder: (context, snapshot) {
        // Je kunt hier ook een nette loader tonen
        if (snapshot.connectionState != ConnectionState.done) {
          return const Scaffold(body: Center(child: CircularProgressIndicator()));
        }

        final host = HostConfig.instance.host; // 'ledclock.local' of custom IP
        return Scaffold(
          appBar: AppBar(
            title: Text('LED Clock (${host})'),
            actions: [
              IconButton(
                icon: const Icon(Icons.settings),
                onPressed: () async {
                  await showDialog(context: context, builder: (_) => const _IpSettingsDialog());
                  setState(() {}); // refresh: titel + panel krijgen nieuwe host
                },
              ),
            ],
          ),
          body: LedClockControlPanel(host: host),
        );
      },
    );
  }
}

/// Eenvoudige dialoog om IP-adres in te stellen of te wissen.
/// Leeg laten = terug naar 'ledclock.local'.
class _IpSettingsDialog extends StatefulWidget {
  const _IpSettingsDialog();

  @override
  State<_IpSettingsDialog> createState() => _IpSettingsDialogState();
}

class _IpSettingsDialogState extends State<_IpSettingsDialog> {
  late final TextEditingController _controller;

  @override
  void initState() {
    super.initState();
    _controller = TextEditingController(text: HostConfig.instance.customIpRaw ?? '');
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  String? _validate(String? value) {
    final v = value?.trim() ?? '';
    if (v.isEmpty) return null; // leeg = ok (betekent: terug naar .local)
    // Heel lichte validatie: geen spaties en geen protocol
    if (v.contains(' ') || v.startsWith('http://') || v.startsWith('https://')) {
      return 'Gebruik alleen host of IP (bijv. 192.168.1.150 of 192.168.1.150:80)';
    }
    return null;
  }

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text('LED Clock adres'),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          const Text(
            'Laat leeg voor "ledclock.local". '
            'Vul IP of hostnaam in om te overschrijven.\n'
            'Voorbeelden: 192.168.1.150  of  192.168.1.150:80',
          ),
          const SizedBox(height: 12),
          TextFormField(
            controller: _controller,
            decoration: const InputDecoration(
              labelText: 'Aangepast IP/host (optioneel)',
              hintText: '192.168.1.150 of 192.168.1.150:80',
              border: OutlineInputBorder(),
            ),
            autovalidateMode: AutovalidateMode.onUserInteraction,
            validator: _validate,
          ),
        ],
      ),
      actions: [
        TextButton(onPressed: () => Navigator.of(context).pop(), child: const Text('Annuleer')),
        TextButton(
          onPressed: () async {
            // Wissen: veld leeg laten
            if ((_controller.text).trim().isEmpty) {
              await HostConfig.instance.setCustomIp(null);
            } else {
              if (_validate(_controller.text) == null) {
                await HostConfig.instance.setCustomIp(_controller.text);
              } else {
                return; // invalid, niet sluiten
              }
            }
            if (context.mounted) Navigator.of(context).pop();
          },
          child: const Text('Opslaan'),
        ),
      ],
    );
  }
}
