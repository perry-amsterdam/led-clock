import 'dart:async';
import 'package:flutter/material.dart';

enum _Seg { a, b, c, d, e, f, g }

const Map<int, Set<_Seg>> _digitMap = {
  0: {_Seg.a, _Seg.b, _Seg.c, _Seg.d, _Seg.e, _Seg.f},
  1: {_Seg.b, _Seg.c},
  2: {_Seg.a, _Seg.b, _Seg.g, _Seg.e, _Seg.d},
  3: {_Seg.a, _Seg.b, _Seg.g, _Seg.c, _Seg.d},
  4: {_Seg.f, _Seg.g, _Seg.b, _Seg.c},
  5: {_Seg.a, _Seg.f, _Seg.g, _Seg.c, _Seg.d},
  6: {_Seg.a, _Seg.f, _Seg.g, _Seg.c, _Seg.d, _Seg.e},
  7: {_Seg.a, _Seg.b, _Seg.c},
  8: {_Seg.a, _Seg.b, _Seg.c, _Seg.d, _Seg.e, _Seg.f, _Seg.g},
  9: {_Seg.a, _Seg.b, _Seg.c, _Seg.d, _Seg.f, _Seg.g},
};

class SevenSegmentDigit extends StatelessWidget {
  const SevenSegmentDigit({
    super.key,
    required this.digit,
    this.height = 80,
    this.onColor = const Color(0xFFFF2B2B),
    this.offColor = const Color(0x22FF2B2B),
    this.glowSigma = 6,
    this.thicknessFactor = 0.12,
    this.gapFactor = 0.08,
    this.background = Colors.transparent,
  });

  final int? digit; // null = blank
  final double height;
  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final double thicknessFactor;
  final double gapFactor;
  final Color background;

  @override
  Widget build(BuildContext context) {
    // typische 7-seg verhouding
    final width = height * 0.58;

    return RepaintBoundary(
      child: CustomPaint(
        size: Size(width, height),
        painter: _SevenSegmentPainter(
          digit: digit,
          onColor: onColor,
          offColor: offColor,
          glowSigma: glowSigma,
          thicknessFactor: thicknessFactor,
          gapFactor: gapFactor,
          background: background,
        ),
      ),
    );
  }
}

class SevenSegmentColon extends StatelessWidget {
  const SevenSegmentColon({
    super.key,
    this.height = 80,
    this.onColor = const Color(0xFFFF2B2B),
    this.offColor = const Color(0x22FF2B2B),
    this.glowSigma = 6,
    this.background = Colors.transparent,
    this.on = true,
  });

  final double height;
  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final Color background;
  final bool on;

  @override
  Widget build(BuildContext context) {
    final width = height * 0.18;
    return RepaintBoundary(
      child: CustomPaint(
        size: Size(width, height),
        painter: _ColonPainter(onColor: onColor, offColor: offColor, glowSigma: glowSigma, background: background, on: on),
      ),
    );
  }
}

class LedClock extends StatefulWidget {
  const LedClock({
    super.key,
    this.height = 80,
    this.onColor = const Color(0xFFFF2B2B),
    this.offColor = const Color(0x22FF2B2B),
    this.glowSigma = 6,
    this.background = Colors.transparent,
    this.spacing = 6,
    this.blinkColon = false,
  });

  final double height;
  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final Color background;
  final double spacing;
  final bool blinkColon;

  @override
  State<LedClock> createState() => _LedClockState();
}

class _LedClockState extends State<LedClock> {
  late Timer _timer;
  late DateTime _now;

  @override
  void initState() {
    super.initState();
    _now = DateTime.now();
    // update netjes op de seconde
    _timer = Timer.periodic(const Duration(milliseconds: 200), (_) {
      final n = DateTime.now();
      if (n.second != _now.second) {
        setState(() => _now = n);
      }
    });
  }

  @override
  void dispose() {
    _timer.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final hh = _now.hour.toString().padLeft(2, '0');
    final mm = _now.minute.toString().padLeft(2, '0');
    final ss = _now.second.toString().padLeft(2, '0');

    final colonOn = !widget.blinkColon || (_now.second % 2 == 0);

    Widget d(String ch) => SevenSegmentDigit(
      digit: int.tryParse(ch),
      height: widget.height,
      onColor: widget.onColor,
      offColor: widget.offColor,
      glowSigma: widget.glowSigma,
      background: widget.background,
    );

    Widget colon() => SevenSegmentColon(
      height: widget.height,
      onColor: widget.onColor,
      offColor: widget.offColor,
      glowSigma: widget.glowSigma,
      background: widget.background,
      on: colonOn,
    );

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        d(hh[0]),
        SizedBox(width: widget.spacing),
        d(hh[1]),
        SizedBox(width: widget.spacing),
        colon(),
        SizedBox(width: widget.spacing),
        d(mm[0]),
        SizedBox(width: widget.spacing),
        d(mm[1]),
        SizedBox(width: widget.spacing),
        colon(),
        SizedBox(width: widget.spacing),
        d(ss[0]),
        SizedBox(width: widget.spacing),
        d(ss[1]),
      ],
    );
  }
}

class _SevenSegmentPainter extends CustomPainter {
  _SevenSegmentPainter({
    required this.digit,
    required this.onColor,
    required this.offColor,
    required this.glowSigma,
    required this.thicknessFactor,
    required this.gapFactor,
    required this.background,
  });

  final int? digit;
  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final double thicknessFactor;
  final double gapFactor;
  final Color background;

  @override
  void paint(Canvas canvas, Size size) {
    if (background.opacity > 0) {
      canvas.drawRect(Offset.zero & size, Paint()..color = background);
    }

    final w = size.width;
    final h = size.height;
    final t = (h * thicknessFactor).clamp(2.0, h / 6);
    final gap = (t * gapFactor).clamp(1.0, t);

    final horizLen = w - 2 * t - 2 * gap;
    final upperH = (h - 3 * t - 4 * gap) / 2;

    final upperY = t + gap;
    final midY = t + gap + upperH + gap;
    final lowerY = midY + t + gap;

    Rect a = Rect.fromLTWH(t + gap, 0, horizLen, t);
    Rect g = Rect.fromLTWH(t + gap, midY, horizLen, t);
    Rect d = Rect.fromLTWH(t + gap, h - t, horizLen, t);

    Rect f = Rect.fromLTWH(0, upperY, t, upperH);
    Rect b = Rect.fromLTWH(w - t, upperY, t, upperH);
    Rect e = Rect.fromLTWH(0, lowerY, t, upperH);
    Rect c = Rect.fromLTWH(w - t, lowerY, t, upperH);

    final rects = <_Seg, Rect>{_Seg.a: a, _Seg.b: b, _Seg.c: c, _Seg.d: d, _Seg.e: e, _Seg.f: f, _Seg.g: g};

    final onSegs = (digit != null) ? (_digitMap[digit!] ?? const <_Seg>{}) : const <_Seg>{};

    for (final seg in _Seg.values) {
      final r = rects[seg]!;
      final isOn = onSegs.contains(seg);
      _drawSegment(canvas, r, isOn);
    }
  }

  void _drawSegment(Canvas canvas, Rect r, bool on) {
    final radius = Radius.circular(r.shortestSide * 0.35);

    final solid = Paint()
      ..color = on ? onColor : offColor
      ..style = PaintingStyle.fill;

    if (on && glowSigma > 0) {
      final glow = Paint()
        ..color = onColor.withOpacity(0.85)
        ..maskFilter = MaskFilter.blur(BlurStyle.normal, glowSigma)
        ..style = PaintingStyle.fill;

      canvas.drawRRect(RRect.fromRectAndRadius(r, radius), glow);
    }

    canvas.drawRRect(RRect.fromRectAndRadius(r, radius), solid);
  }

  @override
  bool shouldRepaint(covariant _SevenSegmentPainter oldDelegate) {
    return oldDelegate.digit != digit ||
        oldDelegate.onColor != onColor ||
        oldDelegate.offColor != offColor ||
        oldDelegate.glowSigma != glowSigma ||
        oldDelegate.thicknessFactor != thicknessFactor ||
        oldDelegate.gapFactor != gapFactor ||
        oldDelegate.background != background;
  }
}

class _ColonPainter extends CustomPainter {
  _ColonPainter({
    required this.onColor,
    required this.offColor,
    required this.glowSigma,
    required this.background,
    required this.on,
  });

  final Color onColor;
  final Color offColor;
  final double glowSigma;
  final Color background;
  final bool on;

  @override
  void paint(Canvas canvas, Size size) {
    if (background.opacity > 0) {
      canvas.drawRect(Offset.zero & size, Paint()..color = background);
    }

    final w = size.width;
    final h = size.height;
    final dotH = h * 0.12;
    final dotW = w * 0.7;
    final x = (w - dotW) / 2;

    Rect top = Rect.fromLTWH(x, h * 0.32 - dotH / 2, dotW, dotH);
    Rect bot = Rect.fromLTWH(x, h * 0.68 - dotH / 2, dotW, dotH);

    final solid = Paint()
      ..color = on ? onColor : offColor
      ..style = PaintingStyle.fill;

    if (on && glowSigma > 0) {
      final glow = Paint()
        ..color = onColor.withOpacity(0.85)
        ..maskFilter = MaskFilter.blur(BlurStyle.normal, glowSigma)
        ..style = PaintingStyle.fill;

      canvas.drawRRect(RRect.fromRectAndRadius(top, Radius.circular(dotH)), glow);
      canvas.drawRRect(RRect.fromRectAndRadius(bot, Radius.circular(dotH)), glow);
    }

    canvas.drawRRect(RRect.fromRectAndRadius(top, Radius.circular(dotH)), solid);
    canvas.drawRRect(RRect.fromRectAndRadius(bot, Radius.circular(dotH)), solid);
  }

  @override
  bool shouldRepaint(covariant _ColonPainter oldDelegate) {
    return oldDelegate.onColor != onColor ||
        oldDelegate.offColor != offColor ||
        oldDelegate.glowSigma != glowSigma ||
        oldDelegate.background != background ||
        oldDelegate.on != on;
  }
}
