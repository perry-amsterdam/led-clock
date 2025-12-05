// ignore_for_file: public_member_api_docs, sort_constructors_first
// LED Clock REST API – Flutter/Dart client (HTTP-only)
//
// Add to pubspec.yaml:
// dependencies:
//   http: ^1.2.2
//
// Usage:
// final api = LedClockApi(host: 'ledclock.local'); // or "192.168.1.50:8080"
// final ping = await api.ping();
// print(ping.uptimeMs);
//
// Generated from: openapi.json (LED Clock REST API 1.6.0)

import 'dart:convert';
import 'package:http/http.dart' as http;

class ApiException implements Exception {
  final int statusCode;
  final String message;
  final String? body;
  ApiException(this.statusCode, this.message, {this.body});
  @override
  String toString() => 'ApiException($statusCode): $message${body != null ? " – $body" : ""}';
}

class LedClockApi {
  /// Host or host:port, e.g. "ledclock.local" or "192.168.1.50:8080"
  final String host;

  final http.Client _client;

  LedClockApi({required this.host, http.Client? client}) : _client = client ?? http.Client();

  Uri _uri(String path, {Map<String, dynamic>? query}) {
    final qp = query == null ? null : query.map((k, v) => MapEntry(k, v?.toString() ?? ''));
    return Uri.http(host, path, qp);
  }

  Map<String, String> get _jsonHeaders => {'Content-Type': 'application/json', 'Accept': 'application/json'};

  Future<T> _handleJson<T>(http.Response r, T Function(dynamic json) parser) async {
    if (r.statusCode < 200 || r.statusCode >= 300) {
      throw ApiException(r.statusCode, 'Request failed', body: r.body);
    }
    if (r.body.isEmpty) {
      throw ApiException(r.statusCode, 'Empty response body');
    }
    final decoded = jsonDecode(utf8.decode(r.bodyBytes));
    return parser(decoded);
  }

  // ---------- Health ----------
  Future<PingResponse> ping() async {
    final r = await _client.get(_uri('/api/ping'), headers: _jsonHeaders);
    return _handleJson(r, (j) => PingResponse.fromJson(j as Map<String, dynamic>));
  }

  // ---------- System ----------
  Future<RebootResponse> reboot() async {
    final r = await _client.post(_uri('/api/system/reboot'), headers: _jsonHeaders);
    return _handleJson(r, (j) => RebootResponse.fromJson(j as Map<String, dynamic>));
  }

  // ---------- Time ----------
  Future<TimezoneInfo> getTimezone() async {
    final r = await _client.get(_uri('/api/timezone'), headers: _jsonHeaders);
    return _handleJson(r, (j) => TimezoneInfo.fromJson(j as Map<String, dynamic>));
  }

  Future<SetTimezoneResult> setTimezone(String timezone) async {
    final body = jsonEncode({'timezone': timezone});
    final r = await _client.post(_uri('/api/timezone'), headers: _jsonHeaders, body: body);
    return _handleJson(r, (j) => SetTimezoneResult.fromJson(j as Map<String, dynamic>));
  }

  Future<ClearTimezoneResult> clearTimezone() async {
    final r = await _client.delete(_uri('/api/timezone'), headers: _jsonHeaders);
    return _handleJson(r, (j) => ClearTimezoneResult.fromJson(j as Map<String, dynamic>));
  }

  Future<List<String>> listTimezones() async {
    final r = await _client.get(_uri('/api/timezones'), headers: _jsonHeaders);
    return _handleJson(r, (j) {
      final map = j as Map<String, dynamic>;
      final list = (map['timezones'] as List<dynamic>? ?? const []);
      return list.map((e) => e.toString()).toList();
    });
  }

  // ---------- Themes ----------
  Future<List<ThemeItem>> listThemes() async {
    final r = await _client.get(_uri('/api/themes'), headers: _jsonHeaders);
    return _handleJson(r, (j) {
      final list = j as List<dynamic>;
      return list.map((e) => ThemeItem.fromJson(e as Map<String, dynamic>)).toList();
    });
  }

  Future<ActiveTheme> getActiveTheme() async {
    final r = await _client.get(_uri('/api/theme'), headers: _jsonHeaders);
    return _handleJson(r, (j) => ActiveTheme.fromJson(j as Map<String, dynamic>));
  }

  /// Sets an active theme by id (see [listThemes]).
  Future<SetThemeResponse> setTheme(String id) async {
    final r = await _client.post(_uri('/api/theme', query: {'id': id}), headers: _jsonHeaders);
    return _handleJson(r, (j) => SetThemeResponse.fromJson(j as Map<String, dynamic>));
  }

  /// Removes stored theme override and activates default.
  Future<DeleteThemeResponse> clearThemeOverride() async {
    final r = await _client.delete(_uri('/api/theme'), headers: _jsonHeaders);
    return _handleJson(r, (j) => DeleteThemeResponse.fromJson(j as Map<String, dynamic>));
  }

  // ---------- Powersave ----------
  /// GET /api/powersave
  /// Returns whether powersave mode is currently enabled.
  Future<bool> getPowersave() async {
    final r = await _client.get(_uri('/api/powersave'), headers: _jsonHeaders);
    return _handleJson(r, (j) {
      final map = j as Map<String, dynamic>;
      return map['enabled'] as bool? ?? false;
    });
  }

  /// POST /api/powersave
  /// Enables or disables powersave mode and returns the resulting state.
  Future<bool> setPowersave(bool enabled) async {
    final body = jsonEncode({'enabled': enabled});
    final r = await _client.post(_uri('/api/powersave'), headers: _jsonHeaders, body: body);
    return _handleJson(r, (j) {
      final map = j as Map<String, dynamic>;
      return map['enabled'] as bool? ?? enabled;
    });
  }

  void close() => _client.close();
}

// -------------------- Models --------------------

// GET /api/ping
class PingResponse {
  final bool pong;
  final int now; // Unix epoch (ms)
  final int uptimeMs;
  final int? heapFree;
  final String? wifiMode;

  PingResponse({required this.pong, required this.now, required this.uptimeMs, this.heapFree, this.wifiMode});

  factory PingResponse.fromJson(Map<String, dynamic> json) => PingResponse(
    pong: json['pong'] as bool? ?? false,
    now: (json['now'] as num?)?.toInt() ?? 0,
    uptimeMs: (json['uptime_ms'] as num?)?.toInt() ?? 0,
    heapFree: (json['heap_free'] as num?)?.toInt(),
    wifiMode: json['wifi_mode'] as String?,
  );

  Map<String, dynamic> toJson() => {
    'pong': pong,
    'now': now,
    'uptime_ms': uptimeMs,
    if (heapFree != null) 'heap_free': heapFree,
    if (wifiMode != null) 'wifi_mode': wifiMode,
  };
}

// POST /api/system/reboot
class RebootResponse {
  final bool? rebooting;
  final String? message;
  RebootResponse({this.rebooting, this.message});
  factory RebootResponse.fromJson(Map<String, dynamic> json) =>
      RebootResponse(rebooting: json['rebooting'] as bool?, message: json['message'] as String?);
  Map<String, dynamic> toJson() => {if (rebooting != null) 'rebooting': rebooting, if (message != null) 'message': message};
}

// GET /api/timezone
class TimezoneInfo {
  final String timezone;
  final int gmtoffset;
  final int dstoffset;
  TimezoneInfo({required this.timezone, required this.gmtoffset, required this.dstoffset});
  factory TimezoneInfo.fromJson(Map<String, dynamic> json) => TimezoneInfo(
    timezone: json['timezone'] as String? ?? 'Etc/UTC',
    gmtoffset: (json['gmtoffset'] as num?)?.toInt() ?? 0,
    dstoffset: (json['dstoffset'] as num?)?.toInt() ?? 0,
  );
  Map<String, dynamic> toJson() => {'timezone': timezone, 'gmtoffset': gmtoffset, 'dstoffset': dstoffset};
}

// POST /api/timezone
class SetTimezoneResult {
  final bool? success;
  final String? message;
  SetTimezoneResult({this.success, this.message});
  factory SetTimezoneResult.fromJson(Map<String, dynamic> json) =>
      SetTimezoneResult(success: json['success'] as bool?, message: json['message'] as String?);
  Map<String, dynamic> toJson() => {if (success != null) 'success': success, if (message != null) 'message': message};
}

// DELETE /api/timezone
class ClearTimezoneResult {
  final bool? success;
  final String? message;
  final String? timezone;
  final int? gmtoffset;
  final int? dstoffset;
  ClearTimezoneResult({this.success, this.message, this.timezone, this.gmtoffset, this.dstoffset});
  factory ClearTimezoneResult.fromJson(Map<String, dynamic> json) => ClearTimezoneResult(
    success: json['success'] as bool?,
    message: json['message'] as String?,
    timezone: json['timezone'] as String?,
    gmtoffset: (json['gmtoffset'] as num?)?.toInt(),
    dstoffset: (json['dstoffset'] as num?)?.toInt(),
  );
  Map<String, dynamic> toJson() => {
    if (success != null) 'success': success,
    if (message != null) 'message': message,
    if (timezone != null) 'timezone': timezone,
    if (gmtoffset != null) 'gmtoffset': gmtoffset,
    if (dstoffset != null) 'dstoffset': dstoffset,
  };
}
// GET /api/timezones -> { timezones: string[] } handled inline

// GET /api/themes
class ThemeItem {
  final String id;
  final String name;
  final bool isDefault;
  final bool isActive;
  ThemeItem({required this.id, required this.name, required this.isDefault, required this.isActive});
  factory ThemeItem.fromJson(Map<String, dynamic> json) => ThemeItem(
    id: json['id'] as String? ?? '',
    name: json['name'] as String? ?? '',
    isDefault: json['is_default'] as bool? ?? false,
    isActive: json['is_active'] as bool? ?? false,
  );
  Map<String, dynamic> toJson() => {'id': id, 'name': name, 'is_default': isDefault, 'is_active': isActive};
}

// GET /api/theme
class ActiveTheme {
  final String activeId;
  final String activeName;
  final bool isDefault;
  final bool hasSavedOverride;
  final String? savedOverrideId;
  ActiveTheme({
    required this.activeId,
    required this.activeName,
    required this.isDefault,
    required this.hasSavedOverride,
    this.savedOverrideId,
  });
  factory ActiveTheme.fromJson(Map<String, dynamic> json) => ActiveTheme(
    activeId: json['active_id'] as String? ?? '',
    activeName: json['active_name'] as String? ?? '',
    isDefault: json['is_default'] as bool? ?? false,
    hasSavedOverride: json['has_saved_override'] as bool? ?? false,
    savedOverrideId: json['saved_override_id'] as String?,
  );
  Map<String, dynamic> toJson() => {
    'active_id': activeId,
    'active_name': activeName,
    'is_default': isDefault,
    'has_saved_override': hasSavedOverride,
    if (savedOverrideId != null) 'saved_override_id': savedOverrideId,
  };
}

// POST /api/theme?id=...
class SetThemeResponse {
  final bool ok;
  final String activeId;
  final String activeName;
  final bool isDefault;
  SetThemeResponse({required this.ok, required this.activeId, required this.activeName, required this.isDefault});
  factory SetThemeResponse.fromJson(Map<String, dynamic> json) => SetThemeResponse(
    ok: json['ok'] as bool? ?? false,
    activeId: json['active_id'] as String? ?? '',
    activeName: json['active_name'] as String? ?? '',
    isDefault: json['is_default'] as bool? ?? false,
  );
  Map<String, dynamic> toJson() => {'ok': ok, 'active_id': activeId, 'active_name': activeName, 'is_default': isDefault};
}

// DELETE /api/theme
class DeleteThemeResponse {
  final bool ok;
  final String activeId;
  final String activeName;
  final bool isDefault;
  DeleteThemeResponse({required this.ok, required this.activeId, required this.activeName, required this.isDefault});
  factory DeleteThemeResponse.fromJson(Map<String, dynamic> json) => DeleteThemeResponse(
    ok: json['ok'] as bool? ?? false,
    activeId: json['active_id'] as String? ?? '',
    activeName: json['active_name'] as String? ?? '',
    isDefault: json['is_default'] as bool? ?? false,
  );
  Map<String, dynamic> toJson() => {'ok': ok, 'active_id': activeId, 'active_name': activeName, 'is_default': isDefault};
}
