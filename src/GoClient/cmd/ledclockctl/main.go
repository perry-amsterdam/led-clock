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
	opSelected := *getTZ || *listTZ || *reboot || (*setTZ != "") || *clearTZ || *listThemes || *getTheme || (*setTheme != "") || *clearTheme

	// --- 0) Ping (alleen wanneer geen gerichte actie is gevraagd)
	if !opSelected {
		ping, err := c.GetApiPingWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/ping: %v", err)
		}
		if *showRaw && ping.Body != nil {
			fmt.Println(string(ping.Body))
		} else if ping.JSON200 != nil {
			nowTime := time.UnixMilli(int64(ping.JSON200.Now)).In(time.Local)
			uptime := time.Duration(ping.JSON200.UptimeMs) * time.Millisecond

			fmt.Printf("Ping Result:\n-----------\n")
			fmt.Printf("pong=%v\n", ping.JSON200.Pong)
			fmt.Printf("now(ms)=%d (%s)\n", ping.JSON200.Now, nowTime.Format("2006-01-02 15:04:05 MST"))
			fmt.Printf("uptime(ms)=%d (%s)\n", ping.JSON200.UptimeMs, formatDuration(uptime))
			fmt.Printf("heap_free=%d\nwifi_mode=%s\n",
				derefInt(ping.JSON200.HeapFree),
				derefString(ping.JSON200.WifiMode),
			)
		}
	}

	// --- 5) Optioneel: reboot
	if *reboot {
		res, err := c.PostApiSystemRebootWithResponse(ctx)
		if err != nil {
			log.Fatalf("POST /api/system/reboot: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		}
		if res.JSON200 != nil && derefBool(res.JSON200.Rebooting) {
			fmt.Println("rebooting:", derefString(res.JSON200.Message))
		} else {
			fmt.Println("reboot request sent (check device)")
		}
	}

	// 3) Optioneel: lijst tijdzones
	if *listTZ {
		ls, err := c.GetApiTimezonesWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/timezones: %v", err)
		}
		if *showRaw && ls.Body != nil {
			fmt.Println(string(ls.Body))
		} else if ls.JSON200 != nil && ls.JSON200.Timezones != nil {
			fmt.Println("supported timezones:")
			for _, z := range *ls.JSON200.Timezones {
				fmt.Println(" -", z)
			}
		}
	}

	// --- 1) Tijdzone zetten
	if *setTZ != "" {
		body := ledclock.PostApiTimezoneJSONRequestBody{Timezone: *setTZ}
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
		fmt.Println("timezone updated:", derefString(res.JSON200.Message))
	}

	// --- 1b) Tijdzone verwijderen (reset)
	if *clearTZ {
		res, err := c.DeleteApiTimezoneWithResponse(ctx)
		if err != nil {
			log.Fatalf("DELETE /api/timezone: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil && derefBool(res.JSON200.Success) {
			msg := derefString(res.JSON200.Message)
			tz := derefString(res.JSON200.Timezone)
			fmt.Printf("timezone cleared: %s\n", msg)
			if tz != "" {
				fmt.Printf("active timezone now: %s (utc_offset_sec=%d)\n",
					tz, res.JSON200.UtcOffsetSec)
			}
		} else {
			fmt.Println("timezone clear request sent (check device)")
		}
	}

	// --- Tijdzone: alleen huidige TZ opvragen
	if *getTZ {
		res, err := c.GetApiTimezoneWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/timezone: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil {
			fmt.Println(res.JSON200.Timezone)
		} else {
			fmt.Println("kon huidige tijdzone niet ophalen")
		}
		return
	}

	// --- Theme: alleen huidige theme opvragen
	if *getTheme {
		res, err := c.GetApiThemeWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/theme: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil {
			fmt.Printf("active theme: %s (%s)\n", res.JSON200.ActiveName, res.JSON200.ActiveId)
			if res.JSON200.HasSavedOverride {
				if res.JSON200.SavedOverrideId != nil {
					fmt.Printf("saved override id: %s\n", *res.JSON200.SavedOverrideId)
				} else {
					fmt.Println("saved override aanwezig")
				}
			}
			if res.JSON200.IsDefault {
				fmt.Println("note: actief theme is het standaard theme")
			}
		} else {
			fmt.Println("kon actief theme niet ophalen")
		}
		return
	}


	// --- 2) Themes: lijst tonen
	if *listThemes {
		ls, err := c.GetApiThemesWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/themes: %v", err)
		}
		if *showRaw && ls.Body != nil {
			fmt.Println(string(ls.Body))
		} else if ls.JSON200 != nil {
			fmt.Println("themes:")
			for _, t := range *ls.JSON200 {
				activeMark := " "
				if t.IsActive {
					activeMark = "*"
				}
				defaultMark := " "
				if t.IsDefault {
					defaultMark = "d"
				}
				// Voorbeeld: [* d] clock_classic (id=clock_classic)
				fmt.Printf("[%s %s] %s (id=%s)\n", activeMark, defaultMark, t.Name, t.Id)
			}
			fmt.Println("\nLegenda: * = actief, d = default")
		} else {
			fmt.Println("geen themes gevonden")
		}
	}

	// --- 3) Theme zetten via id
	if *setTheme != "" {
		res, err := c.PostApiThemeWithResponse(ctx, &ledclock.PostApiThemeParams{Id: *setTheme})
		if err != nil {
			log.Fatalf("POST /api/theme?id=...: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil && res.JSON200.Ok {
			fmt.Printf("theme set: %s (%s)\n", res.JSON200.ActiveName, res.JSON200.ActiveId)
			if res.JSON200.IsDefault {
				fmt.Println("actief theme is het standaard theme")
			}
		} else {
			fmt.Println("kon theme niet zetten (controleer id)")
		}
	}

	// --- 4) Theme resetten naar default
	if *clearTheme {
		res, err := c.DeleteApiThemeWithResponse(ctx)
		if err != nil {
			log.Fatalf("DELETE /api/theme: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil && res.JSON200.Ok {
			fmt.Printf("theme reset naar default: %s (%s)\n", res.JSON200.ActiveName, res.JSON200.ActiveId)
		} else {
			fmt.Println("kon theme niet resetten")
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

