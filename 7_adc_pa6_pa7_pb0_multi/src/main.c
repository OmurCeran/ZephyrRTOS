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
/*Channel configuration: PA6(IN1), PA7(IN7), PB0(IN8) */
static struct adc_channel_cfg channel1_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = 1,
    .differential = 0
};
static struct adc_channel_cfg channel2_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = 7,
    .differential = 0
};
static struct adc_channel_cfg channel3_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = 8,
    .differential = 0
};

static uint16_t sample_buffer;
/*Resuable sequence object for "one channel per read"*/
static struct adc_sequence sequence = {
    .channels = BIT(1) | BIT(7) | BIT(8), /*channel bitmap */
    .buffer = &sample_buffer,
    .buffer_size = sizeof(sample_buffer),
    .resolution = 12, /*12-bit sampling*/
};

static int adc_read_with_channel(uint8_t channel_id, uint16_t *buffer)
{
    sequence.channels = BIT(channel_id);
    int retval= adc_read(adc_dev, &sequence);
    if (retval < 0) {
        printk("Failed to read ADC channel %u: %d\n", channel_id, retval);
        return retval;
    }
    *buffer = sample_buffer;
    return 0;
}

int main()
{
    if(!device_is_ready(adc_dev)) {
        printk("ADC device not ready\n");
        return 0;
    }
    if (adc_channel_setup(adc_dev, &channel1_cfg) || 
        adc_channel_setup(adc_dev, &channel2_cfg) || 
        adc_channel_setup(adc_dev, &channel3_cfg))    {
        printk("Failed to setup ADC channel\n");
        return 0;
    }
    while(1) {
        uint16_t raw_data6 = 0, raw_data7 = 0, raw_data8 = 0;
        
        /*read ADC values from different channels*/
        int retval1 = adc_read_with_channel(1, &raw_data6);
        int retval2 = adc_read_with_channel(7, &raw_data7);
        int retval3 = adc_read_with_channel(8, &raw_data8);

        /*Check return value adc read */
        if (retval1 < 0 || retval2 < 0 || retval3 < 0) {
            printk("Failed to read one or more ADC channels\n");
        } else {
            printk("ADC readings: PA6=%u, PA7=%u, PB0=%u\n", raw_data6, raw_data7, raw_data8);
        }
        k_msleep(1000);
    }

    return 0;
}