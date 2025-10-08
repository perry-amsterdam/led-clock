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
	reboot := flag.Bool("reboot", false, "Reboot het device")
	timeout := flag.Duration("timeout", 5*time.Second, "HTTP timeout")
	flag.Parse()

	httpClient := &http.Client{Timeout: *timeout}

	c, err := ledclock.NewClientWithResponses(*base, ledclock.WithHTTPClient(httpClient))
	if err != nil {
		log.Fatalf("init client: %v", err)
	}

	ctx, cancel := context.WithTimeout(context.Background(), *timeout)
	defer cancel()

	// 1) Ping
	ping, err := c.GetApiPingWithResponse(ctx)
	if err != nil {
		log.Fatalf("GET /api/ping: %v", err)
	}
	if ping.JSON200 != nil {
		fmt.Printf("Ping Result:\n-----------\n");
		fmt.Printf("pong=%v\nrnow(ms)=%d\nuptime(ms)=%d\nheap_free=%d\nwifi_mode=%s\n",
			ping.JSON200.Pong,
			ping.JSON200.Now,        // int64
			ping.JSON200.UptimeMs,   // int64
			derefInt(ping.JSON200.HeapFree),
			derefString(ping.JSON200.WifiMode),
		)
	}

	// 2) Huidige TZ
	tz, err := c.GetApiTimezoneWithResponse(ctx)
	if err != nil {
		log.Fatalf("GET /api/timezone: %v", err)
	}
	if tz.JSON200 != nil {
		fmt.Printf("timezone=%s utc_offset_sec=%d\n", tz.JSON200.Timezone, tz.JSON200.UtcOffsetSec)
	}

	// 3) Optioneel: TZ zetten
	if *setTZ != "" {
		body := ledclock.PostApiTimezoneJSONRequestBody{Timezone: *setTZ}
		res, err := c.PostApiTimezoneWithResponse(ctx, body)
		if err != nil {
			log.Fatalf("POST /api/timezone: %v", err)
		}
		if res.JSON200 == nil || !derefBool(res.JSON200.Success) {
			log.Fatalf("timezone update failed (status %s)", res.Status())
		}
		fmt.Println("timezone updated:", derefString(res.JSON200.Message))
	}

	// 4) Optioneel: lijst tijdzones
	if *listTZ {
		ls, err := c.GetApiTimezonesWithResponse(ctx)
		if err != nil {
			log.Fatalf("GET /api/timezones: %v", err)
		}
		if ls.JSON200 != nil && ls.JSON200.Timezones != nil {
			fmt.Println("supported timezones:")
			for _, z := range *ls.JSON200.Timezones {
				fmt.Println(" -", z)
			}
		}
	}

	// 5) Optioneel: reboot
	if *reboot {
		res, err := c.PostApiSystemRebootWithResponse(ctx)
		if err != nil {
			log.Fatalf("POST /api/system/reboot: %v", err)
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
