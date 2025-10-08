package main

import (
	"context"
	"flag"
	"fmt"
	"log"
	"net/http"
	"time"

	// ↘️ Zet dit op je eigen modulepad, bv:
	// ledclock "github.com/perry-couprie/GoClient/gen"
	ledclock "GoClient/gen"
)

func main() {
	base := flag.String("base", "http://ledclock.local", "Base URL van de ESP32 API")
	setTZ := flag.String("set-tz", "", "Stel tijdzone in (IANA, bv. Europe/Amsterdam)")
	listTZ := flag.Bool("list-tz", false, "Toon ondersteunde tijdzones")
	getTZ := flag.Bool("get-tz", false, "Toon huidige tijdzone en stop")
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

	// Bepaal of er expliciete acties gevraagd zijn
	opSelected := *getTZ || *listTZ || *reboot || (*setTZ != "")

	// 0) Alleen huidige TZ opvragen en tonen
	if *getTZ {
		res, err := c.GetApiTimezoneWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/timezone: %v", err)
		}
		if *showRaw && res.Body != nil {
			fmt.Println(string(res.Body))
		} else if res.JSON200 != nil {
			// Alleen de naam tonen zoals gevraagd in vorige stap
			fmt.Println(res.JSON200.Timezone)
		} else {
			fmt.Println("kon huidige tijdzone niet ophalen")
		}
		return
	}

	// 1) Ping
	if !opSelected {

		ping, err := c.GetApiPingWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/ping: %v", err)
		}
		if *showRaw && ping.Body != nil {
			fmt.Println(string(ping.Body))
		} else if ping.JSON200 != nil {
			fmt.Printf("Ping Result:\n-----------\n")
			fmt.Printf("pong=%v\nnow(ms)=%d\nuptime(ms)=%d\nheap_free=%d\nwifi_mode=%s\n",
				ping.JSON200.Pong,
				ping.JSON200.Now,
				ping.JSON200.UptimeMs,
				derefInt(ping.JSON200.HeapFree),
				derefString(ping.JSON200.WifiMode),
			)
		}
	}

	// 2) Optioneel: TZ zetten
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

	// 4) Optioneel: reboot
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
