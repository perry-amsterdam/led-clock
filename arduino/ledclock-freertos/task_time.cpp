#include <Arduino.h>
#include "rtos.h"
#include "net_time.h"
#include "globals.h"
#include "hal_time_freertos.h"
#include <Preferences.h>          // ⟵ nieuw

extern String g_timezoneIANA;     // ⟵ als deze al in globals staat, is dit genoeg

void task_time(void*)
{
    // Wait for WiFi
    vTaskDelay(pdMS_TO_TICKS(100));
    LOG_STACK_WATERMARK("time:init");
    rtos_wait_bits(EVT_WIFI_UP);
    LOG_STACK_WATERMARK("time:wifi");

    // 1) TZ uit NVS proberen te lezen
    bool haveSavedTz = false;
    {
        Preferences p;
        if (p.begin("sys", true)) {                 // read-only
            String tz = p.getString("tz", "");
            p.end();
            if (tz.length() > 0) {
                g_timezoneIANA = tz;                // bewaar in global (wordt al door API gebruikt)
                setenv("TZ", tz.c_str(), 1);        // runtime TZ instellen
                tzset();
                haveSavedTz = true;
            }
        }
    }

    // 2) NTP & TZ setup
    //    - Als we al een geldige TZ hebben: alleen tijd syncen (geen online TZ-lookup).
    //    - Als niet: doe de volledige internet-setup (inclusief TZ-lookup).
    bool ok = false;
    if (haveSavedTz) {
        ok = setupTimeFromInternet(false);          // aanname: false = “geen online TZ nodig”
    } else {
        ok = setupTimeFromInternet(true);           // true = “mag TZ online bepalen”
    }

    if (ok) {
        xEventGroupSetBits(g_sysEvents, EVT_TIME_READY);
        LOG_STACK_WATERMARK("time:ntp");
    }

    // 3) Periodieke tijds-onderhoud/sync
    for(;;) {
        netTimeMaintain();
        hal_delay_ms(1000);
    }
}
