
// led_clock_api.dart
import 'dart:convert';
import 'package:http/http.dart' as http;

class PingResponse {
  final String message;
  PingResponse(this.message);
  factory PingResponse.fromJson(Map<String, dynamic> j) =>
      PingResponse(j['message']?.toString() ?? 'pong');
}

class ActiveTheme {
  final String name;
  ActiveTheme(this.name);
  factory ActiveTheme.fromJson(Map<String, dynamic> j) =>
      ActiveTheme(j['name']?.toString() ?? '');
}

class ThemeItem {
  final String name;
  ThemeItem(this.name);
  factory ThemeItem.fromJson(Map<String, dynamic> j) =>
      ThemeItem(j['name']?.toString() ?? '');
}

class TimezoneInfo {
  final String timezone;
  TimezoneInfo(this.timezone);
  factory TimezoneInfo.fromJson(Map<String, dynamic> j) =>
      TimezoneInfo(j['timezone']?.toString() ?? 'UTC');
}

class LedClockApi {
  final String host;
  final http.Client _client;
  LedClockApi({required this.host, http.Client? client})
      : _client = client ?? http.Client();

  Uri _uri(String path, {Map<String, dynamic>? query}) {
    final qp = query == null
        ? null
        : query.map((k, v) => MapEntry(k, v?.toString() ?? ''));
    return Uri.http(host, path, qp);
  }

  Map<String, String> get _jsonHeaders =>
      {'Content-Type': 'application/json', 'Accept': 'application/json'};

  Future<PingResponse> ping() async {
    final res = await _client.get(_uri('/api/ping'));
    if (res.statusCode >= 400) {
      throw Exception('Ping failed: ${res.statusCode} ${res.body}');
    }
    final body = jsonDecode(res.body) as Map<String, dynamic>;
    return PingResponse.fromJson(body);
  }

  Future<TimezoneInfo> getTimezone() async {
    final res = await _client.get(_uri('/api/timezone'));
    if (res.statusCode >= 400) {
      throw Exception('Get timezone failed: ${res.statusCode} ${res.body}');
    }
    return TimezoneInfo.fromJson(jsonDecode(res.body));
  }

  Future<List<String>> listTimezones() async {
    final res = await _client.get(_uri('/api/timezones'));
    if (res.statusCode >= 400) {
      throw Exception('List timezones failed: ${res.statusCode} ${res.body}');
    }
    final data = jsonDecode(res.body);
    if (data is List) {
      return data.map((e) => e.toString()).toList();
    }
    return const ['UTC'];
  }

  Future<List<ThemeItem>> listThemes() async {
    final res = await _client.get(_uri('/api/themes'));
    if (res.statusCode >= 400) {
      throw Exception('List themes failed: ${res.statusCode} ${res.body}');
    }
    final data = jsonDecode(res.body);
    if (data is List) {
      return data
          .map((e) => ThemeItem.fromJson((e as Map).cast<String, dynamic>()))
          .toList();
    }
    return const <ThemeItem>[];
  }

  Future<ActiveTheme> getActiveTheme() async {
    final res = await _client.get(_uri('/api/themes/active'));
    if (res.statusCode >= 400) {
      throw Exception('Get active theme failed: ${res.statusCode} ${res.body}');
    }
    return ActiveTheme.fromJson(jsonDecode(res.body));
  }

  Future<void> setActiveTheme(String name) async {
    final res = await _client.post(
      _uri('/api/themes/active'),
      headers: _jsonHeaders,
      body: jsonEncode({'name': name}),
    );
    if (res.statusCode >= 400) {
      throw Exception('Set active theme failed: ${res.statusCode} ${res.body}');
    }
  }

  Future<void> setTimezone(String tz) async {
    final res = await _client.post(
      _uri('/api/timezone'),
      headers: _jsonHeaders,
      body: jsonEncode({'timezone': tz}),
    );
    if (res.statusCode >= 400) {
      throw Exception('Set timezone failed: ${res.statusCode} ${res.body}');
    }
  }

  void close() {
    _client.close();
  }
}
