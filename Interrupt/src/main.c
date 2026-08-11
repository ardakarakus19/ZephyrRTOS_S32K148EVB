#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

static const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));
static const struct pwm_dt_spec pwm_led = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
static const struct gpio_dt_spec buton = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

#define POT_CHANNEL 28

uint16_t adc_val;
struct adc_sequence sequence = {
    .channels = BIT(POT_CHANNEL),
    .buffer = &adc_val,
    .buffer_size = sizeof(adc_val),
    .resolution = 12,
};

volatile bool sistem_aktif = true;
static struct gpio_callback buton_cb;

/* İŞÇİ FONKSİYONU */
void adc_pwm_isleyicisi(struct k_work *work)
{
    if(!sistem_aktif)
    {
        pwm_set(pwm_led.dev, pwm_led.channel, 1000000, 0, pwm_led.flags);
        return;
    }

    int err = adc_read(adc_dev, &sequence);
    
    if(err == 0)
    {
        uint32_t periyot = 1000000;
        uint32_t pulse = ((uint64_t)adc_val * periyot) / 4095;
        pwm_set(pwm_led.dev, pwm_led.channel, periyot, pulse, pwm_led.flags);
    } 
    else 
    {
        /* Hatayı sessizce geçiştirmemek için ekrana yazdırıyoruz */
        printk("ADC Okuma Hatası: %d\n", err);
    }
}
K_WORK_DEFINE(adc_pwm_gorevi, adc_pwm_isleyicisi);

/* TIMER FONKSİYONU */
void timer_tetikleyici(struct k_timer *timer_id)
{
    k_work_submit(&adc_pwm_gorevi);
} 
K_TIMER_DEFINE(benim_timer, timer_tetikleyici, NULL);

/* INTERRUPT FONKSİYONU */
void buton_basildi_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    sistem_aktif = !sistem_aktif;

    if (sistem_aktif) {
        printk(">>> Sistem AKTIF! Sensör okumasi basladi.\n");
    } else {
        printk(">>> Sistem DURDURULDU! (Acil Kesme - Kill Switch)\n");
    }
}

/* ANA FONKSİYON */
int main(void)
{
    /* \n eklendi, artık ekrana basılacak */
    printk("Sistem başlatılıyor...\n"); 

    if (!device_is_ready(adc_dev)) {
        printk("HATA: ADC donanımı hazır değil!\n");
        return 0;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL, 
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = POT_CHANNEL,
        .differential     = 0
    };

    /* KRİTİK DÜZELTME: S32K148 için ADC Referans Fallback Mekanizması */
    int err = adc_channel_setup(adc_dev, &channel_cfg);
    if (err < 0) {
        channel_cfg.reference = ADC_REF_EXTERNAL0;
        err = adc_channel_setup(adc_dev, &channel_cfg);
        if (err < 0) {
            printk("HATA: ADC Kanalı kurulamadı! Hata Kodu: %d\n", err);
            return 0; 
        }
    }

    if (!gpio_is_ready_dt(&buton)) {
        printk("HATA: Buton donanımı hazır değil!\n");
        return 0;
    }
    
    gpio_pin_configure_dt(&buton, GPIO_INPUT);
    gpio_pin_interrupt_configure_dt(&buton, GPIO_INT_EDGE_TO_ACTIVE);
    
    gpio_init_callback(&buton_cb, buton_basildi_isr, BIT(buton.pin));
    gpio_add_callback(buton.port, &buton_cb);

    printk("Her şey hazır! Timer başlatılıyor...\n");
    k_timer_start(&benim_timer, K_MSEC(10), K_MSEC(10));

    while (1) {
        k_msleep(1000); 
    }
    
    return 0;
}