#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/adc.h>

#define ADC_NODE DT_NODELABEL(adc1)
#if !DT_NODE_HAS_STATUS(ADC_NODE, okay)
#error "Unsupported board: adc1 devicetree alias is not defined"
#endif
/*Device get for node */
static const struct device * const adc_dev = DEVICE_DT_GET(ADC_NODE);
/*Channel configuration */
static struct adc_channel_cfg channel1_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = 1,
    .differential = 0
};

static uint16_t sample_buffer;

static struct adc_sequence sequence = {
    .channels = BIT(1), /*channel bitmap */
    .buffer = &sample_buffer,
    .buffer_size = sizeof(sample_buffer),
    .resolution = 12, /*12-bit sampling*/
};

int main()
{
    if(!device_is_ready(adc_dev)) {
        printk("ADC device not ready\n");
        return 0;
    }
    int ret = adc_channel_setup(adc_dev, &channel1_cfg);
    if (ret < 0) {
        printk("Failed to setup ADC channel: %d\n", ret);
        return 0;
    }
    while(1) {
        ret = adc_read(adc_dev, &sequence);
        if (ret < 0) {
            printk("Failed to read ADC: %d\n", ret);
        } else {
            uint16_t raw_data = sample_buffer;
            printk("ADC reading: %u\n", raw_data);
        }
        k_msleep(1000);
    }

    return 0;
}