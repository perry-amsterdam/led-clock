package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"time"

	ledclock "GoClient/gen"
)

func main() {
	base := flag.String("base", "http://ledclock.local", "Base URL van de ESP32 API")

	// Tijdzone
	setTZ := flag.String("set-tz", "", "Stel tijdzone in (IANA, bv. Europe/Amsterdam)")
	listTZ := flag.Bool("list-tz", false, "Toon ondersteunde tijdzones")
	getTZ := flag.Bool("get-tz", false, "Toon huidige tijdzone en stop")
	clearTZ := flag.Bool("clear-tz", false, "Verwijder de ingestelde tijdzone (fallback naar standaard)")

	// Themes
	listThemes := flag.Bool("list-themes", false, "Toon alle themes (id, naam, actief, default)")
	getTheme := flag.Bool("get-theme", false, "Toon het actieve theme (id en naam)")
	setTheme := flag.String("set-theme", "", "Activeer theme via id (zie --list-themes)")
	clearTheme := flag.Bool("clear-theme", false, "Herstel naar het standaard theme")

	// Powersave
	getPS := flag.Bool("get-powersave", false, "Toon de huidige powersave-status")
	setPS := flag.String("set-powersave", "", "Zet powersave: on/off")
	clearPS := flag.Bool("clear-powersave", false, "Reset powersave naar standaard")

	// Overig
	reboot := flag.Bool("reboot", false, "Reboot het device")
	timeout := flag.Duration("timeout", 5*time.Second, "HTTP timeout")
	showRaw := flag.Bool("raw", false, "Toon raw JSON van API responses")
	flag.Parse()

	httpClient := &http.Client{Timeout: *timeout}

	c, err := ledclock.NewClientWithResponses(*base, ledclock.WithHTTPClient(httpClient))
	if err != nil {
		log.Fatalf("init client: %v", err)
	}

	ctx, cancel := context.WithTimeout(context.Background(), *timeout)
	defer cancel()

	// Is er een expliciete actie?
	opSelected := *getTZ ||
		*listTZ ||
		(*setTZ != "") ||
		*clearTZ ||
		*listThemes ||
		*getTheme ||
		(*setTheme != "") ||
		*clearTheme ||
		*getPS || (*setPS != "") || *clearPS ||
		*reboot

	// --- 0) Ping (alleen wanneer geen gerichte actie is gevraagd)
	if !opSelected {
		ping, err := c.GetApiPingWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/ping: %v", err)
		}
		if *showRaw && ping.Body != nil {
			fmt.Println(string(ping.Body))
			return
		}

		if ping.JSON200 == nil {
			log.Fatalf("ping faalde, geen JSON200 (status %s)", ping.Status())
		}

		p := ping.JSON200
		fmt.Printf("Pong: %v\n", p.Pong)
		fmt.Printf("Nu:   %d (ms since epoch)\n", p.Now)
		fmt.Printf("Up:   %s\n", formatDuration(time.Duration(p.UptimeMs)*time.Millisecond))
		if p.HeapFree != nil {
			fmt.Printf("Free heap: %d bytes\n", *p.HeapFree)
		}
		if p.WifiMode != nil {
			fmt.Printf("WiFi mode: %s\n", *p.WifiMode)
		}
		return
	}

	// --- TIMEZONE: lijst tonen
	if *listTZ {
		resp, err := c.GetApiTimezonesWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/timezones: %v", err)
		}
		if *showRaw && resp.Body != nil {
			fmt.Println(string(resp.Body))
			return
		}
		if resp.JSON200 == nil {
			log.Fatalf("kon tijdzones niet ophalen (status %s)", resp.Status())
		}
		fmt.Println("Ondersteunde tijdzones:")
		for _, tz := range *resp.JSON200.Timezones {
			fmt.Printf(" - %s\n", tz)
		}
		return
	}

	// --- TIMEZONE: huidige tijdzone opvragen
	if *getTZ {
		resp, err := c.GetApiTimezoneWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/timezone: %v", err)
		}
		if *showRaw && resp.Body != nil {
			fmt.Println(string(resp.Body))
			return
		}
		if resp.JSON200 == nil {
			log.Fatalf("kon tijdzone niet ophalen (status %s)", resp.Status())
		}
		tz := resp.JSON200
		fmt.Printf("Huidige tijdzone: %s\n", tz.Timezone)
		fmt.Printf("GMT offset:       %d s\n", tz.Gmtoffset)
		fmt.Printf("DST offset:       %d s\n", tz.Dstoffset)
		return
	}

	// --- TIMEZONE: nieuwe tijdzone instellen
	if *setTZ != "" {
		body := ledclock.PostApiTimezoneJSONRequestBody{
			Timezone: *setTZ,
		}
		res, err := c.PostApiTimezoneWithResponse(ctx, body)
		if err != nil {
			log.Fatalf("POST /api/timezone: %v", err)
		}

		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		}
		if res.JSON200 == nil || !derefBool(res.JSON200.Success) {
			log.Fatalf("timezone update failed (status %s)", res.Status())
		}
		fmt.Println("Tijdzone succesvol ingesteld.")
		return
	}

	// --- TIMEZONE: tijdzone resetten
	if *clearTZ {
		res, err := c.DeleteApiTimezoneWithResponse(ctx)
		if err != nil {
			log.Fatalf("DELETE /api/timezone: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
			return
		}
		if res.JSON200 == nil {
			log.Fatalf("kon tijdzone niet resetten (status %s)", res.Status())
		}
		fmt.Println("Tijdzone gereset naar standaard.")
		fmt.Printf("Actieve TZ: %s (gmtoffset=%d, dstoffset=%d)\n",
			derefString(res.JSON200.Timezone),
			derefInt(res.JSON200.Gmtoffset),
			derefInt(res.JSON200.Dstoffset))
		return
	}

	// --- THEMES: lijst tonen
	if *listThemes {
		res, err := c.GetApiThemesWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/themes: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
			return
		}
		if res.JSON200 == nil {
			log.Fatalf("kon themes niet ophalen (status %s)", res.Status())
		}
		fmt.Println("Beschikbare themes:")
		for _, t := range *res.JSON200 {
			active := ""
			if t.IsActive {
				active = " (actief)"
			}
			def := ""
			if t.IsDefault {
				def = " (default)"
			}
			fmt.Printf(" - %s (%s)%s%s\n", t.Id, t.Name, active, def)
		}
		return
	}

	// --- THEMES: actief theme opvragen
	if *getTheme {
		res, err := c.GetApiThemeWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/theme: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
			return
		}
		if res.JSON200 == nil {
			log.Fatalf("kon actief theme niet ophalen (status %s)", res.Status())
		}
		t := res.JSON200
		fmt.Printf("Actief theme: %s (%s)\n", t.ActiveId, t.ActiveName)
		if t.IsDefault {
			fmt.Println("Dit is het default theme.")
		} else {
			fmt.Println("Dit is een override theme.")
		}
		if t.HasSavedOverride {
			fmt.Printf("Er is een opgeslagen override: %s\n", derefString(t.SavedOverrideId))
		}
		return
	}

	// --- THEMES: theme instellen
	if *setTheme != "" {
		res, err := c.PostApiThemeWithResponse(ctx, &ledclock.PostApiThemeParams{Id: *setTheme})
		if err != nil {
			log.Fatalf("POST /api/theme: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
			return
		}
		if res.JSON200 == nil || !res.JSON200.Ok {
			log.Fatalf("kon theme niet instellen (status %s)", res.Status())
		}
		t := res.JSON200
		fmt.Printf("Theme ingesteld: %s (%s)\n", t.ActiveId, t.ActiveName)
		if t.IsDefault {
			fmt.Println("Dit is nu het default theme.")
		}
		return
	}

	// --- THEMES: theme override verwijderen
	if *clearTheme {
		res, err := c.DeleteApiThemeWithResponse(ctx)
		if err != nil {
			log.Fatalf("DELETE /api/theme: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
			return
		}
		if res.JSON200 == nil || !res.JSON200.Ok {
			log.Fatalf("kon theme override niet verwijderen (status %s)", res.Status())
		}
		t := res.JSON200
		fmt.Println("Theme override verwijderd; default theme actief.")
		fmt.Printf("Actief theme: %s (%s)\n", t.ActiveId, t.ActiveName)
		return
	}

	// --- POWERSAVE: huidige status opvragen
	if *getPS {
		res, err := c.GetApiPowersaveWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/powersave: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil {
			status := "OFF"
			if res.JSON200.Enabled {
				status = "ON"
			}
			fmt.Printf("Powersave: %s\n", status)
			if res.JSON200.Message != nil && *res.JSON200.Message != "" {
				fmt.Printf("Message: %s\n", *res.JSON200.Message)
			}
		} else {
			fmt.Println("kon powersave-status niet ophalen")
		}
		return
	}

	// --- POWERSAVE: powersave-modus zetten
	if *setPS != "" {
		var v bool
		if *setPS == "on" {
			v = true
		} else if *setPS == "off" {
			v = false
		} else {
			log.Fatalf("--set-powersave verwacht 'on' of 'off'")
		}

		body := ledclock.PostApiPowersaveJSONRequestBody{Enabled: v}
		res, err := c.PostApiPowersaveWithResponse(ctx, body)
		if err != nil {
			log.Fatalf("POST /api/powersave: %v", err)
		}

		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil {
			status := "OFF"
			if res.JSON200.Enabled {
				status = "ON"
			}
			fmt.Printf("Powersave updated: %s\n", status)
			if res.JSON200.Message != nil && *res.JSON200.Message != "" {
				fmt.Printf("Message: %s\n", *res.JSON200.Message)
			}
		} else {
			fmt.Println("powersave update failed")
		}
	}

	// --- POWERSAVE: resetten
	if *clearPS {
		res, err := c.DeleteApiPowersaveWithResponse(ctx)
		if err != nil {
			log.Fatalf("DELETE /api/powersave: %v", err)
		}

		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil {
			status := "OFF"
			if res.JSON200.Enabled {
				status = "ON"
			}
			fmt.Printf("powersave reset naar standaard (nu: %s)\n", status)
			if res.JSON200.Message != nil && *res.JSON200.Message != "" {
				fmt.Printf("Message: %s\n", *res.JSON200.Message)
			}
		} else {
			fmt.Println("kon powersave niet resetten")
		}
	}
}

func derefInt(p *int) int {
	if p == nil {
		return 0
	}
	return *p
}

func derefString(p *string) string {
	if p == nil {
		return ""
	}
	return *p
}

func derefBool(p *bool) bool {
	if p == nil {
		return false
	}
	return *p
}

func formatDuration(d time.Duration) string {
	seconds := int64(d.Seconds())

	years := seconds / (365 * 24 * 3600)
	days := (seconds % (365 * 24 * 3600)) / (24 * 3600)
	hours := (seconds % (24 * 3600)) / 3600
	minutes := (seconds % 3600) / 60
	secs := seconds % 60

	if years > 0 {
		return fmt.Sprintf("%dy %dd %02dh %02dm %02ds", years, days, hours, minutes, secs)
	} else if days > 0 {
		return fmt.Sprintf("%dd %02dh %02dm %02ds", days, hours, minutes, secs)
	}
	return fmt.Sprintf("%02dh %02dm %02ds", hours, minutes, secs)
}
