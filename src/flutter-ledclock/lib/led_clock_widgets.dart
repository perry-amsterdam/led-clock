
// led_clock_widgets.dart
import 'dart:async';
import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'led_clock_api.dart';
import 'theme/ikea_theme.dart';

class LedClockControlPanel extends StatefulWidget {
  LedClockControlPanel({super.key});

  @override
  State<LedClockControlPanel> createState() => _LedClockControlPanelState();
}

class _LedClockControlPanelState extends State<LedClockControlPanel> {
  final TextEditingController _hostController = TextEditingController();
  late LedClockApi api;

  String _host = 'ledclock.local';
  String status = '—';
  bool busy = false;

  PingResponse? ping;
  TimezoneInfo? tz;
  List<String> tzList = const [];
  List<ThemeItem> themes = const [];
  ActiveTheme? active;

  @override
  void initState() {
    super.initState();
    _init();
  }

  Future<void> _init() async {
    final prefs = await SharedPreferences.getInstance();
    _host = prefs.getString('ledclock_host') ?? _host;
    _hostController.text = _host;
    api = LedClockApi(host: _host);
    unawaited(_refreshAll());
  }

  Future<void> _setBusy(bool v) async {
    if (!mounted) return;
    setState(() => busy = v);
  }

  Future<void> _run(Future<void> Function() task) async {
    await _setBusy(true);
    try {
      await task();
    } finally {
      await _setBusy(false);
    }
  }

  Future<void> _refreshAll() async {
    await _run(() async {
      ping = await api.ping();
      tz = await api.getTimezone();
      tzList = await api.listTimezones();
      themes = await api.listThemes();
      active = await api.getActiveTheme();
      if (!mounted) return;
      setState(() {});
    });
  }

  @override
  void dispose() {
    api.close();
    _hostController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Row(
          children: [
            Container(
              width: 12,
              height: 12,
              decoration: BoxDecoration(
                color: Theme.of(context).colorScheme.secondary,
                shape: BoxShape.circle,
              ),
            ),
            SizedBox(width: 10),
            Text('LED Clock', style: TextStyle(fontWeight: FontWeight.w800)),
            SizedBox(width: 10),
            Text('Control', style: TextStyle(fontWeight: FontWeight.w400)),
          ],
        ),
        bottom: PreferredSize(
          preferredSize: Size.fromHeight(4),
          child: SizedBox(
            height: 4,
            child: ColoredBox(color: Theme.of(context).colorScheme.secondary),
          ),
        ),
      ),
      body: AbsorbPointer(
        absorbing: busy,
        child: Padding(
          padding: const EdgeInsets.all(16),
          child: ListView(
            children: [
              // Host row
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16),
                  child: Row(
                    children: [
                      Text('Host:', style: TextStyle(fontWeight: FontWeight.bold)),
                      SizedBox(width: 12),
                      Expanded(
                        child: TextField(
                          controller: _hostController,
                          decoration: InputDecoration(
                            hintText: 'ledclock.local of 192.168.x.y',
                          ),
                          onSubmitted: (_) => _connect(),
                        ),
                      ),
                      SizedBox(width: 12),
                      IkeaCtaButton(onPressed: _connect, leading: Icons.link, child: const Text('Connect')),
                    ],
                  ),
                ),
              ),
              SizedBox(height: 16),

              // Status row
              Card(
                child: ListTile(
                  leading: Icon(Icons.info_outline),
                  title: Text('Status'),
                  subtitle: Text(status),
                  trailing: busy ? SizedBox(
                    width: 20, height: 20, child: CircularProgressIndicator(strokeWidth: 2),
                  ) : IconButton(
                    icon: Icon(Icons.refresh),
                    onPressed: _refreshAll,
                  ),
                ),
              ),
              SizedBox(height: 16),

              // Ping row
              Card(
                child: ListTile(
                  leading: Icon(Icons.wifi_tethering),
                  title: Text('Ping'),
                  subtitle: Text(ping?.message ?? '—'),
                  trailing: OutlinedButton(onPressed: () async {
                      await _run(() async {
                        ping = await api.ping();
                        if (!mounted) return;
                        setState(() {});
                      });
                    },
                    child: Text('Ping'),
                  ),
                ),
              ),
              SizedBox(height: 16),

              // Timezone row
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text('Timezone', style: TextStyle(fontWeight: FontWeight.bold)),
                      SizedBox(height: 8),
                      DropdownButton<String>(
                        value: tz?.timezone,
                        isExpanded: true,
                        hint: Text('Select timezone'),
                        items: tzList.map((e) => DropdownMenuItem(
                          value: e,
                          child: Text(e),
                        )).toList(),
                        onChanged: (val) async {
                          if (val == null) return;
                          await _run(() async {
                            await api.setTimezone(val);
                            tz = TimezoneInfo(val);
                            if (!mounted) return;
                            setState(() {});
                          });
                        },
                      ),
                    ],
                  ),
                ),
              ),
              SizedBox(height: 16),

              // Themes row
              Card(
                child: Padding(
                  padding: const EdgeInsets.all(16),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      Text('Themes', style: TextStyle(fontWeight: FontWeight.bold)),
                      SizedBox(height: 8),
                      Wrap(
                        spacing: 8,
                        runSpacing: 8,
                        children: [
                          for (final t in themes)
                            ChoiceChip(
                              label: Text(t.name),
                              selected: active?.name == t.name,
                              onSelected: (sel) async {
                                if (!sel) return;
                                await _run(() async {
                                  await api.setActiveTheme(t.name);
                                  active = ActiveTheme(t.name);
                                  if (!mounted) return;
                                  setState(() {});
                                });
                              },
                            ),
                          TextButton(
                            onPressed: () async {
                              await _run(() async {
                                await api.setActiveTheme('default');
                                active = ActiveTheme('default');
                                if (!mounted) return;
                                setState(() {});
                              });
                            }, child: const Text('Use default theme')),
                        ],
                      )
                    ],
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  Future<void> _connect() async {
    final newHost = _hostController.text.trim();
    if (newHost.isEmpty) return;
    setState(() {
      _host = newHost;
      status = 'Connecting to $_host…';
      api = LedClockApi(host: _host);
    });
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('ledclock_host', _host);
    await _refreshAll();
  }
}
