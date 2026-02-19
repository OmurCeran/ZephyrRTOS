/*Pull in the core zephyr headers e.g k_sleep()*/
#include <zephyr/kernel.h>
/*Include Zephyr device model types*/
#include <zephyr/device.h>
/*Access C macros for reading information from the devicetree at build time*/
#include <zephyr/devicetree.h>
/*GPIO driver API*/
#include <zephyr/drivers/gpio.h>

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
    while(1) {
        //5.Toggle the LED state
        printk("Button state: %d\n", gpio_pin_get_dt(&button));
        k_msleep(1000);
        if(gpio_pin_get_dt(&button) == 1) {
            gpio_pin_toggle_dt(&led);
            gpio_pin_toggle_dt(&led1);
            gpio_pin_toggle_dt(&led2);
            gpio_pin_toggle_dt(&led3);
        //6.Sleep for 1000 milliseconds
        k_msleep(1000);
        }
    }

    return 0;
}