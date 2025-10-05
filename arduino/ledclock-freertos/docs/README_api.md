# LED Clock REST API – Documentatie

Deze map bevat de **OpenAPI-specificatie** voor de REST API van het ESP32 LED Clock project.  
De specificatie staat in [`openapi.json`](./openapi.json).

## Inhoud

- **Ping endpoint**
  - `GET /api/ping` → geeft een JSON met `pong: true` en statusvelden zoals uptime, heap memory en Wi-Fi modus.

Voorbeeld response:
```json
{
  "pong": true,
  "now": 1733246400000,
  "uptime_ms": 52341,
  "heap_free": 187264,
  "wifi_mode": "AP"
}
```

## Gebruiken van de specificatie

### 1. In Postman
1. Open Postman.
2. Ga naar **File → Import**.
3. Selecteer `openapi.json`.
4. Postman maakt automatisch een collectie met de endpoints.

### 2. In Swagger UI (lokaal)
Je kunt de API live bekijken in een browser met Swagger UI:

1. Ga naar https://editor.swagger.io/
2. Open `openapi.json` via **File → Import File**.
3. Je ziet de API-documentatie en kunt direct `/api/ping` uitproberen.

### 3. Client code genereren
**Voorbeeld (Go client via oapi-codegen):**
```bash
oapi-codegen -generate types,client -o ledclock_client.gen.go -package ledclock docs/openapi.json
```

### 4. Curl testen (zonder tooling)
Als je verbonden bent met de ESP32 (bijv. via AP-modus op `ledclock.local`):
```bash
curl http://ledclock.local/api/ping
```

## Roadmap

- [ ] Toevoegen van authenticatie (API-key of JWT)
- [ ] Endpoints voor lampjes, animaties, instellingen
- [ ] Versiebeheer (`v1`, `v2` …)
