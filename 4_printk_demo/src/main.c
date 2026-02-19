#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main()
{

    while(1) {
        char buf[64];
        printk("\r\nHello Zephyr!\r\n");
        (void)snprintf(buf, sizeof(buf), "Uptime: %lld ms\r\n", k_uptime_get());
        printk("%s", buf);
        k_msleep(1000);
    }

    return 0;
}