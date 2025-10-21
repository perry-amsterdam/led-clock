import 'package:shared_preferences/shared_preferences.dart';

/// Beheert welk host-adres wordt gebruikt voor de LED Clock API.
/// - Standaard: 'ledclock.local'
/// - Als een custom IP is ingesteld: dat IP.
/// Door `init()` te roepen bij app-start worden de prefs ingeladen.
class HostConfig {
  HostConfig._();
  static final HostConfig instance = HostConfig._();

  static const _prefsKey = 'custom_ledclock_ip';
  static const String defaultHost = 'ledclock.local';

  String? _customIp; // onbewerkt (kan null of '' zijn)

  /// Het effectieve host-adres dat de app moet gebruiken.
  /// Voorbeeld: 'ledclock.local' of '192.168.1.150' of '192.168.1.150:80'
  String get host => (_customIp != null && _customIp!.trim().isNotEmpty) ? _customIp!.trim() : defaultHost;

  /// Lees huidige custom IP (kan null/empty zijn).
  String? get customIpRaw => _customIp;

  /// Initialiseer vanuit SharedPreferences.
  Future<void> init() async {
    final prefs = await SharedPreferences.getInstance();
    _customIp = prefs.getString(_prefsKey);
  }

  /// Stel een custom IP-adres in. Lege string of null = wissen (terug naar .local).
  Future<void> setCustomIp(String? value) async {
    final prefs = await SharedPreferences.getInstance();
    final v = value?.trim();
    if (v == null || v.isEmpty) {
      _customIp = null;
      await prefs.remove(_prefsKey);
    } else {
      _customIp = v;
      await prefs.setString(_prefsKey, v);
    }
  }
}
