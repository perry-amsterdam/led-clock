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
import 'led_clock_api.dart';

class LedClockControlPanel extends StatefulWidget {
  final String host;
  const LedClockControlPanel({super.key, required this.host});

  @override
  State<LedClockControlPanel> createState() => _LedClockControlPanelState();
}

class _LedClockControlPanelState extends State<LedClockControlPanel> {
  late final LedClockApi api;
  String status = '—';
  PingResponse? ping;
  TimezoneInfo? tz;
  List<String> tzList = const [];
  List<ThemeItem> themes = const [];
  ActiveTheme? active;
  bool busy = false;

  @override
  void initState() {
    super.initState();
    api = LedClockApi(host: widget.host);
    _refreshAll();
  }

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
    return AbsorbPointer(
      absorbing: busy,
      child: Padding(
        padding: const EdgeInsets.all(16),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const SizedBox(height: 20),
            Row(
              children: [
                const Text('LED Clock Control', style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
                const SizedBox(width: 12),
                if (busy) const SizedBox(width: 16, height: 16, child: CircularProgressIndicator(strokeWidth: 2)),
                const Spacer(),
                Text(status),
                IconButton(tooltip: 'Refresh', onPressed: _refreshAll, icon: const Icon(Icons.refresh)),
              ],
            ),
            const SizedBox(height: 8),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('Health', style: Theme.of(context).textTheme.titleMedium?.copyWith(fontWeight: FontWeight.bold)),
                    const SizedBox(height: 8),
                    Text('Uptime (ms): ${ping?.uptimeMs ?? "—"}'),
                    Text('Heap free: ${ping?.heapFree ?? "—"}'),
                    Text('WiFi mode: ${ping?.wifiMode ?? "—"}'),
                    const SizedBox(height: 8),
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
            const SizedBox(height: 8),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text('Timezone', style: TextStyle(fontWeight: FontWeight.bold)),
                    const SizedBox(height: 8),
                    Text('Current: ${tz?.timezone ?? "—"},
                    const SizedBox(height: 8),
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
            const SizedBox(height: 8),
            Card(
              child: Padding(
                padding: const EdgeInsets.all(12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Wrap(
                      spacing: 8,
                      runSpacing: 8,
                      children: [
                        DropdownButton<String>(
                          value: themes.any((t) => t.id == active?.activeId) ? active?.activeId : null,
                          hint: const Text('Select theme'),
                          items: themes
                              .map(
                                (t) => DropdownMenuItem<String>(
                                  value: t.id,
                                  child: Text(t.name + (t.isDefault ? ' • default' : '') + (t.isActive ? ' • active' : '')),
                                ),
                              )
                              .toList(),
                          onChanged: (id) {
                            if (id == null) return;
                            _run(() async {
                              await api.setTheme(id);
                              active = await api.getActiveTheme();
                              setState(() {});
                            });
                          },
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
