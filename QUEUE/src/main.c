#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

/*DONANIM TANIMLARI*/
static const struct device *adc_dev= DEVICE_DT_GET(DT_NODELABEL(adc0));
static const struct pwm_dt_spec pwm_led= PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
#define POT_CHANNEL 28

/*KUYRUK AYARI*/
K_MSGQ_DEFINE(adc_kuyrugu,sizeof(uint16_t),10,4);

/*ADC AYARI*/
void adc_okuma_thread(void *arg1,void *arg2, void *arg3)
{
    struct adc_channel_cfg channel_cfg={

        .gain=ADC_GAIN_1,
        .reference=ADC_REF_INTERNAL,
        .acquisition_time=ADC_ACQ_TIME_DEFAULT,
        .channel_id=POT_CHANNEL,
        .differential=0,
    };

    uint16_t okunan_deger;

    struct adc_sequence sequence={

        .channels=BIT(POT_CHANNEL),
        .buffer=&okunan_deger,
        .buffer_size=sizeof(okunan_deger),
        .resolution=12,
    };

    while(1)
    {
        if(adc_read(adc_dev,&sequence)==0)
        {
            k_msgq_put(&adc_kuyrugu,&okunan_deger,K_MSEC(10));
        }

        k_msleep(50);
    }
}

void pwm_kontrol_thread(void *arg1, void *arg2, void *arg3)
{

    uint16_t gelen_deger;
    uint32_t periyot = 1000000;

    while (1)
    {
        if(k_msgq_get(&adc_kuyrugu,&gelen_deger,K_FOREVER)==0)
        {
            uint32_t pulse = ((uint64_t)gelen_deger * periyot) / 4095;
            pwm_set(pwm_led.dev, pwm_led.channel, periyot, pulse, pwm_led.flags);
        }
    }
    
}

K_THREAD_DEFINE(adc_tid, 1024, adc_okuma_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(pwm_tid, 1024, pwm_kontrol_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    while (1) {
        /* Ana döngü artık tamamen uyur, bütün şovu yukarıdaki Thread'ler kendi aralarında yapar */
        k_sleep(K_FOREVER);
    }
    return 0;
}