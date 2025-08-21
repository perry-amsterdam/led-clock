#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/mdns_responder.h>

void main(void) {
    printk("🚀 LED Clock firmware started on Zephyr!\n");

    // Hier kun je later Wi-Fi connectie, mDNS en API server toevoegen
    // Voor nu alleen een hello world
    while (1) {
        printk("LED Clock running...\n");
        k_sleep(K_SECONDS(5));
    }
}
