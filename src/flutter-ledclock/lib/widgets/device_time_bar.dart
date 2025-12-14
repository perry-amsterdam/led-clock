import 'dart:async';
import 'package:flutter/material.dart';

import '../led_clock_api.dart';
import 'led_clock.dart'; // bevat SevenSegmentDigit + SevenSegmentColon

class DeviceTimeBar extends StatefulWidget {
  const DeviceTimeBar({
    super.key,
    required this.host,
    this.height = 56,
    this.pollInterval = const Duration(seconds: 5),
    this.onColor = const Color(0xFFFF2B2B),
    this.offColor = const Color(0x22FF2B2B),
    this.glowSigma = 7,
    this.spacing = 8,
    this.blinkColon = false,
  });

  final String host;
  final double height;
  final Duration pollInterval;

  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final double spacing;
  final bool blinkColon;

  @override
  State<DeviceTimeBar> createState() => _DeviceTimeBarState();
}

class _DeviceTimeBarState extends State<DeviceTimeBar> {
  late LedClockApi _api;

  Timer? _pollTimer;
  Timer? _tickTimer;

  DateTime? _deviceBaseLocal; // device tijd (local) op moment van sync
  DateTime? _receivedAtLocal; // wanneer we die ontvangen hebben (local)

  DateTime? get _deviceNow {
    if (_deviceBaseLocal == null || _receivedAtLocal == null) return null;
    final drift = DateTime.now().difference(_receivedAtLocal!);
    return _deviceBaseLocal!.add(drift);
  }

  @override
  void initState() {
    super.initState();
    _setup();
  }

  @override
  void didUpdateWidget(covariant DeviceTimeBar oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.host != widget.host) {
      _setup();
    }
  }

  @override
  void dispose() {
    _pollTimer?.cancel();
    _tickTimer?.cancel();
    super.dispose();
  }

  void _setup() {
    _pollTimer?.cancel();
    _tickTimer?.cancel();

    _api = LedClockApi(host: widget.host);

    _sync();

    // poll device time af en toe (niet elke seconde nodig)
    _pollTimer = Timer.periodic(widget.pollInterval, (_) => _sync());

    // local ticker om de display vloeiend per seconde te laten lopen
    _tickTimer = Timer.periodic(const Duration(milliseconds: 200), (_) {
      if (!mounted) return;
      if (_deviceBaseLocal == null) return;
      setState(() {}); // _deviceNow herberekent
    });
  }

  Future<void> _sync() async {
    try {
      final p = await _api.ping();
      // p.now = unix epoch (ms). Interpreteer als UTC en zet om naar local.
      final deviceUtc = DateTime.fromMillisecondsSinceEpoch(p.now, isUtc: true);
      setState(() {
        _deviceBaseLocal = deviceUtc.toLocal();
        _receivedAtLocal = DateTime.now();
      });
    } catch (_) {
      // Als ping faalt: laat laatste tijd doorlopen (of leeg als er geen sync was)
      // Geen extra UI nodig; je kunt hier eventueel een indicator toevoegen.
    }
  }

  @override
  Widget build(BuildContext context) {
    final now = _deviceNow;

    // knipperen op basis van device-seconden (als bekend)
    final colonOn = !widget.blinkColon || (now != null && now.second.isEven);

    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 10),
      color: Colors.black,
      child: Center(
        child: FittedBox(
          fit: BoxFit.scaleDown,
          child: _SevenSegmentTimeRow(
            time: now,
            height: widget.height,
            onColor: widget.onColor,
            offColor: widget.offColor,
            glowSigma: widget.glowSigma,
            spacing: widget.spacing,
            colonOn: colonOn,
          ),
        ),
      ),
    );
  }
}

class _SevenSegmentTimeRow extends StatelessWidget {
  const _SevenSegmentTimeRow({
    required this.time,
    required this.height,
    required this.onColor,
    required this.offColor,
    required this.glowSigma,
    required this.spacing,
    required this.colonOn,
  });

  final DateTime? time;
  final double height;
  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final double spacing;
  final bool colonOn;

  @override
  Widget build(BuildContext context) {
    String? hh0, hh1, mm0, mm1, ss0, ss1;
    if (time != null) {
      final hh = time!.hour.toString().padLeft(2, '0');
      final mm = time!.minute.toString().padLeft(2, '0');
      final ss = time!.second.toString().padLeft(2, '0');
      hh0 = hh[0];
      hh1 = hh[1];
      mm0 = mm[0];
      mm1 = mm[1];
      ss0 = ss[0];
      ss1 = ss[1];
    }

    Widget d(String? ch) => SevenSegmentDigit(
      digit: ch == null ? null : int.parse(ch),
      height: height,
      onColor: onColor,
      offColor: offColor,
      glowSigma: glowSigma,
      background: Colors.transparent,
    );

    Widget colon() => SevenSegmentColon(
      height: height,
      onColor: onColor,
      offColor: offColor,
      glowSigma: glowSigma,
      background: Colors.transparent,
      on: colonOn,
    );

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        d(hh0),
        SizedBox(width: spacing),
        d(hh1),
        SizedBox(width: spacing),
        colon(),
        SizedBox(width: spacing),
        d(mm0),
        SizedBox(width: spacing),
        d(mm1),
        SizedBox(width: spacing),
        colon(),
        SizedBox(width: spacing),
        d(ss0),
        SizedBox(width: spacing),
        d(ss1),
      ],
    );
  }
}
