#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>

/*Use board's chosen console uart*/
#define UART_NODE DT_CHOSEN(zephyr_console)
//1.Check for uart alias in the devicetree
#if !DT_NODE_HAS_STATUS(UART_NODE, okay)
#error "Unsupported board: uart devicetree alias is not defined"
#endif
//2.Build a typed description of the uart_Node alias
static const struct device *const uart_dev = DEVICE_DT_GET(UART_NODE);

static void uart_tx_str(const char *s) {
    while(*s) {
    /*Send character and iterate to next*/
    uart_poll_out(uart_dev, (unsigned char)*s++);
    }
}

int main()
{
    if(!device_is_ready(uart_dev)) {
        return 0;
    }
    uart_tx_str("\r\nHello Zephyr!\r\n");

    while(1) {
        char buf[64];
        (void)snprintf(buf, sizeof(buf), "Uptime: %lld ms\r\n", k_uptime_get());
        uart_tx_str(buf);
        k_msleep(1000);
    }

    return 0;
}