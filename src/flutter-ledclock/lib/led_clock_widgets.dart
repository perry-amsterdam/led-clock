
// A tiny Flutter widget that exercises the entire LED Clock API.
// Drop this into your app (e.g. lib/led_clock_widgets.dart) and use LedClockControlPanel().
//
// Requires:
//   http: ^1.2.2
//
// Example:
// MaterialApp(home: Scaffold(body: LedClockControlPanel(host: 'ledclock.local')))

import 'dart:async';
import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'led_clock_api.dart';

class LedClockControlPanel extends StatefulWidget {
  final String initialHost;
  const LedClockControlPanel({super.key, required this.initialHost});

  @override
  State<LedClockControlPanel> createState() => _LedClockControlPanelState();
}

class _LedClockControlPanelState extends State<LedClockControlPanel> {
  String _host = 'ledclock.local';
  final TextEditingController _hostController = TextEditingController();
  @override
  void initState() {
    super.initState();
    _host = widget.initialHost;
    _hostController.text = _host;
    api = LedClockApi(host: _host);
    _refreshAll();
  }
  @override
  void didUpdateWidget(covariant LedClockControlPanel oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.initialHost != widget.initialHost) {
      setState(() { 
        _host = widget.initialHost;
        _hostController.text = _host;
        api = LedClockApi(host: _host);
      });
      _refreshAll();
    }
  }

  late final LedClockApi api;
  String status = '—';
  PingResponse? ping;
  TimezoneInfo? tz;
  List<String> tzList = const [];
  List<ThemeItem> themes = const [];
  ActiveTheme? active;
  bool busy = false;
Future<void> _run(Future<void> Function() task) async {
    setState(() => busy = true);
    try {
      await task();
      setState(() => status = 'OK');
    } catch (e) {
      setState(() => status = 'Error: $e');
    } finally {
      setState(() => busy = false);
    }
  }

  Future<void> _refreshAll() async {
    await _run(() async {
      ping = await api.ping();
      tz = await api.getTimezone();
      tzList = await api.listTimezones();
      themes = await api.listThemes();
      active = await api.getActiveTheme();
      setState(() {});
    });
  }

  @override
  void dispose() {
    api.close();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Row(children:[
          Container(width: 12, height: 12, decoration: const BoxDecoration(color: Color(0xFFFFCC00), shape: BoxShape.circle)),
          const SizedBox(width: 10),
          const Text('LED Clock', style: TextStyle(fontWeight: FontWeight.w800)),
          const SizedBox(width: 10),
          const Text('Control', style: TextStyle(fontWeight: FontWeight.w400)),
        ]),
        bottom: const PreferredSize(
          preferredSize: Size.fromHeight(4),
          child: SizedBox(height: 4, child: ColoredBox(color: Color(0xFFFFCC00))),
        ),
      ),
      body: AbsorbPointer(
      absorbing: busy,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: ListView(
          children: [
            // Host input row: allows overriding mDNS with manual IP/host
            Card(child: Padding(padding: const EdgeInsets.all(16), child: Row(
              children: [
                const Text('Host:', style: TextStyle(fontWeight: FontWeight.bold)),
                const SizedBox(width: 12),
                Expanded(
                  child: TextField(
                    controller: _hostController,
                    decoration: const InputDecoration(
                      hintText: 'ledclock.local or 192.168.x.x[:port]',
                      border: OutlineInputBorder(),
                      isDense: true,
                      contentPadding: EdgeInsets.symmetric(horizontal: 8, vertical: 8),
                    ),
                  ),
                ),
                const SizedBox(width: 10),
                ElevatedButton(
                  onPressed: () async {
                    final newHost = _hostController.text.trim();
                    if (newHost.isEmpty) return;
                    setState(() {
                      _host = newHost;
                      status = 'Connecting to ' + _host + '…';
                      api = LedClockApi(host: _host);
                    });
                    final prefs = await SharedPreferences.getInstance();
                    await prefs.setString('ledclock_host', _host);
                    await _refreshAll();
                  },
                  child: const Text('Connect'),
                ),
              ],
            ))),
            const SizedBox(height: 16),
            Row(
              children: [
                const Text('LED Clock Control', style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
                const SizedBox(width: 12),
                if (busy) const SizedBox(width: 16, height: 16, child: CircularProgressIndicator(strokeWidth: 2)),
                const Spacer(),
                Text(status),
                IconButton(
                  tooltip: 'Refresh',
                  onPressed: _refreshAll,
                  icon: const Icon(Icons.refresh),
                )
              ],
            ),
            const SizedBox(height: 10),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text('Health', style: TextStyle(fontWeight: FontWeight.bold)),
                    const SizedBox(height: 10),
                    Text('Uptime (ms): ${ping?.uptimeMs ?? "—"}'),
                    Text('Heap free: ${ping?.heapFree ?? "—"}'),
                    Text('WiFi mode: ${ping?.wifiMode ?? "—"}'),
                    const SizedBox(height: 10),
                    ElevatedButton.icon(
                      onPressed: () => _run(() async {
                        await api.reboot();
                      }),
                      icon: const Icon(Icons.restart_alt),
                      label: const Text('Reboot device'),
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 10),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text('Timezone', style: TextStyle(fontWeight: FontWeight.bold)),
                    const SizedBox(height: 10),
                    Text('Current: ${tz?.timezone ?? "—"} (offset: ${tz?.utcOffsetSec ?? "—"}s)'),
                    const SizedBox(height: 10),
                    Wrap(
                      spacing: 8,
                      runSpacing: 8,
                      children: [
                        DropdownButton<String>(
                          value: tzList.contains(tz?.timezone) ? tz?.timezone : null,
                          hint: const Text('Select timezone'),
                          items: tzList.map((z) => DropdownMenuItem(value: z, child: Text(z))).toList(),
                          onChanged: (z) {
                            if (z == null) return;
                            _run(() async {
                              await api.setTimezone(z);
                              tz = await api.getTimezone();
                              setState(() {});
                            });
                          },
                        ),
                        ElevatedButton(
                          onPressed: () => _run(() async {
                            await api.clearTimezone();
                            tz = await api.getTimezone();
                            setState(() {});
                          }),
                          child: const Text('Clear override'),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ),
            const SizedBox(height: 10),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text('Themes', style: TextStyle(fontWeight: FontWeight.bold)),
                    const SizedBox(height: 10),
                    if (active != null) Text('Active: ${active!.activeName} (${active!.activeId}) ${active!.isDefault ? "(default)" : ""}'),
                    const SizedBox(height: 10),
                    Wrap(
                      spacing: 8,
                      children: [
                        for (final t in themes)
                          OutlinedButton(
                            onPressed: () => _run(() async {
                              await api.setTheme(t.id);
                              active = await api.getActiveTheme();
                              setState(() {});
                            }),
                            child: Text(t.name + (t.isDefault ? ' • default' : '') + (t.isActive ? ' • active' : '')),
                          ),
                        ElevatedButton(
                          onPressed: () => _run(() async {
                            await api.clearThemeOverride();
                            active = await api.getActiveTheme();
                            setState(() {});
                          }),
                          child: const Text('Use default theme'),
                        ),
                      ],
                    ),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
