import 'package:shared_preferences/shared_preferences.dart';

class HostConfig {
  HostConfig._();
  static final HostConfig instance = HostConfig._();

  static const _prefsKey = 'custom_ledclock_ip';
  static const String defaultHost = 'ledclock.local';

  String? _customIp;

  String get host => (_customIp != null && _customIp!.trim().isNotEmpty)
      ? _customIp!.trim()
      : defaultHost;

  Future<void> init() async {
    final prefs = await SharedPreferences.getInstance();
    _customIp = prefs.getString(_prefsKey);
  }

  /// Leeg/null = wissen → terug naar ledclock.local
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

  String? get customIpRaw => _customIp;
}

