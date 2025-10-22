// ignore_for_file: prefer_const_constructors

import 'package:flutter/material.dart';

class SplashScreen extends StatefulWidget {
  const SplashScreen({super.key, required this.appName, required this.makerName, required this.onEnterApp});

  final String appName;
  final String makerName;
  final void Function(BuildContext) onEnterApp;

  @override
  State<SplashScreen> createState() => _SplashScreenState();
}

class _SplashScreenState extends State<SplashScreen> with SingleTickerProviderStateMixin {
  double _opacity = 0.0;

  @override
  void initState() {
    super.initState();
    // Start de fade-in na korte vertraging
    Future.delayed(const Duration(milliseconds: 200), () {
      setState(() => _opacity = 1.0);
    });
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);

    return Scaffold(
      backgroundColor: theme.colorScheme.surface,
      body: SafeArea(
        child: Center(
          child: AnimatedOpacity(
            opacity: _opacity,
            duration: const Duration(seconds: 1),
            curve: Curves.easeInOut,
            child: Padding(
              padding: const EdgeInsets.all(24),
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  // Circle card with Tux
                  Container(
                    width: 160,
                    height: 160,
                    decoration: BoxDecoration(
                      shape: BoxShape.circle,
                      boxShadow: [
                        BoxShadow(
                          blurRadius: 24,
                          spreadRadius: 2,
                          offset: Offset(0, 12),
                          color: Colors.black.withOpacity(0.15),
                        ),
                      ],
                      gradient: RadialGradient(
                        colors: [theme.colorScheme.primary.withOpacity(0.15), theme.colorScheme.primary.withOpacity(0.05)],
                      ),
                    ),
                    child: ClipOval(
                      child: Image.network(
                        'https://upload.wikimedia.org/wikipedia/commons/3/35/Tux.svg',
                        fit: BoxFit.contain,
                        errorBuilder: (context, error, stack) {
                          return Icon(Icons.pets, size: 96, color: theme.colorScheme.primary);
                        },
                      ),
                    ),
                  ),
                  SizedBox(height: 28),
                  Text(
                    widget.appName,
                    textAlign: TextAlign.center,
                    style: theme.textTheme.displaySmall?.copyWith(fontWeight: FontWeight.w800, letterSpacing: -0.5),
                  ),
                  SizedBox(height: 8),
                  Text(
                    'gemaakt door ${widget.makerName}',
                    textAlign: TextAlign.center,
                    style: theme.textTheme.titleMedium?.copyWith(color: theme.colorScheme.onSurface.withOpacity(0.7)),
                  ),
                  SizedBox(height: 32),
                  SizedBox(
                    width: 220,
                    height: 52,
                    child: ElevatedButton.icon(
                      style: ElevatedButton.styleFrom(shape: StadiumBorder(), elevation: 8),
                      onPressed: () => widget.onEnterApp(context),
                      icon: Icon(Icons.play_arrow_rounded),
                      label: Text(
                        'Ga naar de app',
                        style: theme.textTheme.titleMedium?.copyWith(
                          color: theme.colorScheme.onPrimary,
                          fontWeight: FontWeight.w600,
                        ),
                      ),
                    ),
                  ),
                  SizedBox(height: 16),
                  Text(
                    '© ${DateTime.now().year} ${widget.makerName}',
                    style: theme.textTheme.bodySmall?.copyWith(color: theme.colorScheme.onSurface.withOpacity(0.5)),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
