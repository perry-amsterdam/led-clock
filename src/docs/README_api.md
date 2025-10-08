# LED Clock REST API – Full Documentation

This directory contains the **OpenAPI specification** for the ESP32 LED Clock REST API.
The full specification is available in [`openapi.json`](./openapi.json) (version **1.3.0**).

---

## Overview

The LED Clock API provides system management and time-related endpoints for an ESP32-based LED clock running FreeRTOS. It enables you to:

* Check device health and uptime
* Reboot the system remotely
* Get or set the current timezone
* Retrieve a list of supported IANA timezones

All responses are in JSON format.

---

## Endpoints

### 🩺 `GET /api/ping`

**Purpose:** Check if the device is online and responding.

**Response Example:**

```json
{
  "pong": true,
  "now": 1733246400000,
  "uptime_ms": 52341,
  "heap_free": 187264,
  "wifi_mode": "AP"
}
```

**Fields:**

* `pong`: Always `true` if device is reachable.
* `now`: Current time in Unix epoch (ms).
* `uptime_ms`: Time since boot in milliseconds.
* `heap_free`: Free heap memory in bytes.
* `wifi_mode`: Current Wi-Fi mode (e.g., `AP`, `STA`).

---

### 🔁 `POST /api/system/reboot`

**Purpose:** Trigger a soft reboot of the ESP32 device.

**Response Example:**

```json
{
  "rebooting": true,
  "message": "Rebooting system..."
}
```

**Behavior:** The system will reboot within a few seconds after returning this response.

---

### 🌍 `GET /api/timezone`

**Purpose:** Retrieve the current timezone configuration.

**Response Example:**

```json
{
  "timezone": "Europe/Amsterdam",
  "utc_offset_sec": 7200
}
```

**Fields:**

* `timezone`: Current IANA timezone string.
* `utc_offset_sec`: UTC offset in seconds (including DST if applicable).

---

### 🕓 `POST /api/timezone`

**Purpose:** Update the timezone setting (IANA format only).

**Request Body:**

```json
{
  "timezone": "Europe/Amsterdam"
}
```

**Response Example:**

```json
{
  "success": true,
  "message": "Timezone updated successfully"
}
```

**Notes:**

* This changes only the timezone configuration. The clock time continues to be managed by NTP/RTC.

---

### 🌐 `GET /api/timezones`

**Purpose:** Retrieve a list of supported IANA timezones.

**Response Example:**

```json
{
  "timezones": [
    "Europe/Amsterdam",
    "America/New_York",
    "Asia/Tokyo"
  ]
}
```

**Notes:**

* This is a compact list that can be expanded in future versions.

---

## Using the Specification

### 1. In Postman

1. Open **Postman**.
2. Go to **File → Import**.
3. Select `openapi.json`.
4. Postman automatically creates a collection with all endpoints.

### 2. In Swagger UI (locally or online)

You can visualize and test the API live in your browser:

1. Go to [https://editor.swagger.io/](https://editor.swagger.io/)
2. Open `openapi.json` via **File → Import File**.
3. The full API documentation will load, allowing you to test each endpoint interactively.

### 3. Generate Client Code

**Example (Go client via `oapi-codegen`):**

```bash
oapi-codegen -generate types,client -o ledclock_client.gen.go -package ledclock docs/openapi.json
```

**Supported generators:** Go (`oapi-codegen`), Python (`openapi-python-client`), TypeScript (`openapi-typescript`), etc.

### 4. Test with curl

If your ESP32 is reachable (e.g., in AP mode on `ledclock.local`):

```bash
curl http://ledclock.local/api/ping
curl -X POST http://ledclock.local/api/system/reboot
curl http://ledclock.local/api/timezone
curl -X POST -H "Content-Type: application/json" -d '{"timezone": "Europe/Amsterdam"}' http://ledclock.local/api/timezone
curl http://ledclock.local/api/timezones
```

---

## Roadmap

* [ ] Add authentication (API key or JWT)
* [ ] Introduce endpoints for LED animations and brightness control
* [ ] Add `/api/time` for real-time NTP sync information
* [ ] Implement versioned API structure (`/v1`, `/v2`, ...)

---

**Maintainer:** Perry Couprie
**Project:** ESP32 LED Clock with FreeRTOS and WS2812B

---

*This API and documentation are licensed under the MIT License.*
