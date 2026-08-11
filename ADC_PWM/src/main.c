#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h> /* Yeni: PWM kütüphanesi */
#include <zephyr/sys/printk.h>

static const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

/* Yeni: S32K148 kartındaki Kırmızı LED'in PWM donanımını çekiyoruz */
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

#define POT_CHANNEL 28

int main(void)
{
    k_msleep(2000);
    
    printk("\n===========================================\n");
    printk("--- S32K148 ADC Okuma ve PWM Testi ---\n");
    printk("===========================================\n");

    /* ADC Kontrolü */
    if (!device_is_ready(adc_dev)) {
        printk("HATA: adc0 donanimi baslatilamadi!\n");
        return 0; 
    }

    /* Yeni: PWM Kontrolü - Donanım hazır mı? */
    if (!pwm_is_ready_dt(&pwm_led)) {
        printk("HATA: PWM donanimi (FTM4) baslatilamadi!\n");
        return 0;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL, 
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = POT_CHANNEL,
        .differential     = 0
    };

    int err = adc_channel_setup(adc_dev, &channel_cfg);
    if (err < 0) {
        channel_cfg.reference = ADC_REF_EXTERNAL0;
        err = adc_channel_setup(adc_dev, &channel_cfg);
        if (err < 0) {
            printk("HATA: Kanal kurulamadi!\n");
            return 0; 
        }
    }

    uint16_t adc_val;
    struct adc_sequence sequence = {
        .channels    = BIT(POT_CHANNEL),
        .buffer      = &adc_val,
        .buffer_size = sizeof(adc_val),
        .resolution  = 12,
    };

    printk("Sistem Hazir. Potansiyometre LED'i kontrol ediyor...\n\n");

    while (1) {
        err = adc_read(adc_dev, &sequence);
        
        if (err < 0) {
            printk("ADC Okuma Hatasi: %d\r\n", err);
        } else {
            
            uint32_t yeni_periyot = 1000000; 
            uint32_t pulse = ((uint64_t)adc_val * yeni_periyot) / 4095;

            /* 2. PWM'i Ayarla: */
            pwm_set(pwm_led.dev, pwm_led.channel, yeni_periyot, pulse, pwm_led.flags);

        }
        
        k_msleep(100);
    }
    
    return 0;
}