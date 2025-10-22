
# Opstartscherm (SplashScreen) toevoegen

1. Zet `splash_screen.dart` in `lib/` van je Flutter project.
2. Open `main.dart` en stel `home:` in op `SplashScreen`:
   ```dart
   home: SplashScreen(
     appName: 'LED Klok',
     makerName: 'Jouw Naam',
     onEnterApp: () {
       // Navigeer naar je bestaande startscherm:
       // Navigator.of(context).pushReplacement(
       //   MaterialPageRoute(builder: (_) => const YourHomeWidget()),
       // );
     },
   ),
   ```
3. Als je named routes gebruikt:
   ```dart
   onEnterApp: () => Navigator.of(context).pushReplacementNamed('/home'),
   ```
4. (Aanrader) Voeg de Tux-afbeelding toe als asset in `pubspec.yaml` voor offline gebruik.
   Voor nu gebruikt het scherm een publieke URL.

Aangemaakte bestanden:
- `splash_screen.dart`
- `main_with_splash.dart`
