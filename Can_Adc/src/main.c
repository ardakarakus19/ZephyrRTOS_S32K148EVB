#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/can.h>
#include <zephyr/sys/printk.h>

/* ADC Cihazı ve Kanalı */
static const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));
#define POT_CHANNEL 28

/* CAN Cihazı */
const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

int main(void)
{
   
    can_set_mode(can_dev, CAN_MODE_NORMAL);
    can_start(can_dev);


    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL, 
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = POT_CHANNEL,
        .differential     = 0
    };

    adc_channel_setup(adc_dev, &channel_cfg);
    
    uint16_t adc_val;
    struct adc_sequence sequence = {
        .channels    = BIT(POT_CHANNEL),
        .buffer      = &adc_val,
        .buffer_size = sizeof(adc_val),
        .resolution  = 12,
    };

    /* CAN Çerçevesi (Frame) Şablonu */
    struct can_frame frame = {
        .id = 0x123,
        .flags = 0,
        .dlc = 2, /* Verimiz 16-bit (2 byte) olduğu için dlc'yi 2 yaptık */
    };

    while (1) {
        /* Potansiyometre değerini oku */
        adc_read(adc_dev, &sequence);
        
        /* 16 bitlik adc_val değerini 2 byte'a bölüp CAN data dizisine diziyoruz */
        frame.data[0] = (adc_val >> 8) & 0xFF; /* İlk 8 bit (MSB) */
        frame.data[1] = adc_val & 0xFF;        /* Son 8 bit (LSB) */

        can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
            
        
        
        /* candump ekranını sel gibi boğmamak için 100ms bekliyoruz */
        k_msleep(100);
    }
    
    return 0;
}