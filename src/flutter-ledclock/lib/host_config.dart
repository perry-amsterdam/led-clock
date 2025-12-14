import 'dart:io'; // voor InternetAddress.lookup
import 'package:shared_preferences/shared_preferences.dart';

class HostConfig {
  HostConfig._();
  static final HostConfig instance = HostConfig._();

  static const String defaultHost = 'ledclock.local';
  static const String _prefsKeyCustomIp = 'custom_ledclock_ip';
  static const String _prefsKeyResolvedIp = 'resolved_ledclock_ip';

  String? _customIp;
  String? _resolvedIp;

  /// Host die de app daadwerkelijk gebruikt:
  /// - eerst een handmatig ingesteld IP (indien aanwezig),
  /// - anders het gecachte IP van ledclock.local (indien bekend),
  /// - anders de hostname 'ledclock.local' als fallback.
  String get host {
    if (_customIp != null && _customIp!.trim().isNotEmpty) {
      return _customIp!.trim();
    }
    if (_resolvedIp != null && _resolvedIp!.isNotEmpty) {
      return _resolvedIp!;
    }
    return defaultHost;
  }

  /// Ruwe custom IP-string zoals opgeslagen (kan null of leeg zijn).
  /// Dit wordt gebruikt door de IP settings dialog om de huidige waarde te tonen.
  String? get customIpRaw => _customIp;

  /// Initialiseer configuratie uit SharedPreferences.
  /// Laadt custom IP en, als die niet bestaat, een eventueel gecached IP.
  /// Als er helemaal niets is, wordt een eerste resolve van ledclock.local gedaan.
  Future<void> init() async {
    final prefs = await SharedPreferences.getInstance();
    _customIp = prefs.getString(_prefsKeyCustomIp);
    _resolvedIp = prefs.getString(_prefsKeyResolvedIp);

    // Als er geen custom IP én geen cached IP is, één keer resolven
    if (_customIp == null && _resolvedIp == null) {
      await _resolveAndCache();
    }
  }

  /// Resolve ledclock.local naar een IP-adres en cache dat.
  Future<void> _resolveAndCache() async {
    try {
      final addresses = await InternetAddress.lookup(defaultHost);
      if (addresses.isEmpty) return;

      final ip = addresses.firstWhere((a) => a.type == InternetAddressType.IPv4, orElse: () => addresses.first);

      _resolvedIp = ip.address;

      final prefs = await SharedPreferences.getInstance();
      await prefs.setString(_prefsKeyResolvedIp, _resolvedIp!);
    } catch (_) {
      // Als resolven faalt laten we _resolvedIp null;
      // dan wordt gewoon defaultHost gebruikt als fallback.
    }
  }

  /// Forceer een her-resolve van ledclock.local.
  Future<void> refreshResolution() async {
    final prefs = await SharedPreferences.getInstance();
    _resolvedIp = null;
    await prefs.remove(_prefsKeyResolvedIp);
    await _resolveAndCache();
  }

  /// Stel een custom IP-adres in.
  /// - Leeg of null = terug naar automatische .local modus (met IP-cache).
  /// - Niet-leeg = altijd dit adres gebruiken.
  Future<void> setCustomIp(String? value) async {
    final prefs = await SharedPreferences.getInstance();
    final normalized = value?.trim();

    if (normalized == null || normalized.isEmpty) {
      // Terug naar automatische .local modus
      _customIp = null;
      await prefs.remove(_prefsKeyCustomIp);

      // en meteen opnieuw het IP van ledclock.local resolven
      await refreshResolution();
    } else {
      _customIp = normalized;
      await prefs.setString(_prefsKeyCustomIp, _customIp!);
    }
  }
}
