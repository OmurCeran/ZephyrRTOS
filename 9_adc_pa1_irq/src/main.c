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

/*Forward decleration : work item function that resubmits adc_read()*/
static void adc_resubmit_work_handler(struct k_work *work);
/*Define a kernel work item we can submit from ISR callback*/
static K_WORK_DEFINE(adc_resubmit_work, adc_resubmit_work_handler);


/*Channel configuration */
static struct adc_channel_cfg channel1_cfg = {
    .gain = ADC_GAIN_1,
    .reference = ADC_REF_INTERNAL,
    .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    .channel_id = 1,
    .differential = 0
};

static uint16_t sample_buffer;

/*Callback definition for adc irq*/
static enum adc_action adc_callback(const struct device *dev,
                                    const struct adc_sequence *sequence,
                                    uint16_t sampling_index)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(sequence);
    ARG_UNUSED(sampling_index);

    uint16_t sensor_value = sample_buffer;
    printk("ADC callback called for value: %u\n", sensor_value);

    /*TODO: Resubmit the next conversion from thread context*/
    k_work_submit(&adc_resubmit_work);

    /*tell the driver , it is done*/
    return ADC_ACTION_FINISH;
}

/*Callback definition*/
static struct adc_sequence_options sequence_options = {
    .interval_us = 0 /*sample as fast as possible*/,
    .extra_samplings = 0, /*one-shot , we will resubmit work continuously*/
    .user_data = NULL,
    .callback = adc_callback
};

static struct adc_sequence sequence = {
    .channels = BIT(1), /*channel bitmap */
    .buffer = &sample_buffer,
    .buffer_size = sizeof(sample_buffer),
    .resolution = 12, /*12-bit sampling*/
    .options = &sequence_options /*options is added*/
};
/*work handler*/
static void adc_resubmit_work_handler(struct k_work *work)
{
    int received= adc_read(adc_dev, &sequence);
    ARG_UNUSED(work);
    ARG_UNUSED(received);
}

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
    /*Start the first ADC conversion, it is for first time , after this one , it will work continuosly as interrupt*/
    int received = adc_read(adc_dev, &sequence);
    if (received < 0) {
        printk("Failed to start ADC conversion: %d\n", received);
        return 0;
    }
    /*Starting info*/
    printk("ADC conversion started successfully\n");
    while(1) {

        k_sleep(K_FOREVER);
    }

    return 0;
}