#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>

/*Use board's chosen console uart*/
#define UART_NODE DT_CHOSEN(zephyr_console)
//1.Check for uart alias in the devicetree
#if !DT_NODE_HAS_STATUS(UART_NODE, okay)
#error "Unsupported board: uart devicetree alias is not defined"
#endif
//1.Check for led0 alias in the devicetree
#if !DT_NODE_HAS_STATUS(DT_ALIAS(led0), okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
//2.Build a typed descriptiopnn of the led0 alias
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
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
    if(!device_is_ready(uart_dev) || !device_is_ready(led.port)) {
        return 0;
    }

        if(gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) != 0) {
        return 0;
    }
    uart_tx_str("\r\nHello Zephyr!\r\n");

    unsigned char ch;
    while(1) {
        /*Poll for a character to be received*/
        int received = uart_poll_in(uart_dev, &ch);
        if(received == 0) {
            if(ch == 'O' || ch == 'o') {
                uart_tx_str("Received O, turning on LED\r\n");
                /*Echo back written character*/ 
               // uart_poll_out(uart_dev, ch);
                gpio_pin_set_dt(&led, 1);
            } else if(ch == 'F' || ch == 'f') {
                uart_tx_str("Received F, turning off LED\r\n");
                gpio_pin_set_dt(&led, 0);
            } else {
                uart_tx_str("Received unknown character\r\n");
            }
        }
        k_msleep(1000);
    }

    return 0;
}