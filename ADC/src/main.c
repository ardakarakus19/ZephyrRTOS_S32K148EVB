#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

static const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

#define POT_CHANNEL 28

int main(void)
{

    /* Cihaz hazır değilse döngüye girmeden programı güvenlice bitir */
    if (!device_is_ready(adc_dev)) {
        printk("HATA: adc0 donanimi baslatilamadi!\n");
        if (adc_dev != NULL && adc_dev->state != NULL) {
            printk("-> Sürücü Hata Kodu (init_res): %d\n", adc_dev->state->init_res);
        }
        return 0; 
    }

    printk("adc0 HAZIR! Kanal ayarlaniyor...\n");

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL, 
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = POT_CHANNEL,
        .differential     = 0
    };

    int err = adc_channel_setup(adc_dev, &channel_cfg);
    if (err < 0) {
        printk("UYARI: INTERNAL referans calismadi (%d). EXTERNAL deneniyor...\n", err);
        
        channel_cfg.reference = ADC_REF_EXTERNAL0;
        err = adc_channel_setup(adc_dev, &channel_cfg);
        if (err < 0) {
            printk("HATA: Kanal kesinlikle kurulamadi (%d)!\n", err);
            return 0; 
        }
    }

    printk("Kanal 28 Kuruldu. Okuma basliyor...\n\n");

    uint16_t adc_val;
    struct adc_sequence sequence = {
        .channels    = BIT(POT_CHANNEL),
        .buffer      = &adc_val,
        .buffer_size = sizeof(adc_val),
        .resolution  = 12,
    };

    while (1) {
        err = adc_read(adc_dev, &sequence);
        if (err < 0) {
            printk("ADC Okuma Hatasi: %d\n", err);
        } else {
            printk("%d\n", adc_val);
        }
        k_msleep(100);
    }
    
    return 0;
}