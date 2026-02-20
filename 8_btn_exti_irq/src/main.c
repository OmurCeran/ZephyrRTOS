#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>


//1.Check for led0 alias in the devicetree
#if !DT_NODE_HAS_STATUS(DT_ALIAS(led0), okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
//1.Check for led0 alias in the devicetree
#if !DT_NODE_HAS_STATUS(DT_ALIAS(led1), okay)
#error "Unsupported board: led1 devicetree alias is not defined"
#endif
//1.Check for led2 alias in the devicetree
#if !DT_NODE_HAS_STATUS(DT_ALIAS(led2), okay)
#error "Unsupported board: led2 devicetree alias is not defined"
#endif
//1.Check for led3 alias in the devicetree
#if !DT_NODE_HAS_STATUS(DT_ALIAS(led3), okay)
#error "Unsupported board: led3 devicetree alias is not defined"
#endif
#if !DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
//2.Build a typed descriptiopnn of the led0 alias
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, gpio_port_pins_t pins) {
   /*this function is called from IRQ*/
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    printk("Button pressed! Pin: %d\n", button.pin);
    gpio_pin_toggle_dt(&led);
    gpio_pin_toggle_dt(&led1);
    gpio_pin_toggle_dt(&led2);
    gpio_pin_toggle_dt(&led3);
}

int main()
{
    //3.Make sure the GPIO device is ready
    if (!device_is_ready(led.port) || 
        !device_is_ready(led1.port) || 
        !device_is_ready(led2.port) || 
        !device_is_ready(led3.port) || 
        !device_is_ready(button.port)) {
        return 0;
    }
    //4.Configure the pin as output push-pull, and ensure it is inactive to start
    if(gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE) != 0 || 
       gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE) != 0 || 
       gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE) != 0 || 
       gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE) != 0) {
        return 0;
    }
    //4.Configure the button as input pull-up
    if(gpio_pin_configure_dt(&button, GPIO_INPUT) != 0) {
        return 0;
    }
    /*Callback init and interrupt configure */
    gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);
    if(gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_RISING) != 0) {
        return 0;
    }
    printk("Press the button (PC13) to toggle the LEDs!\n");
    while(1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}