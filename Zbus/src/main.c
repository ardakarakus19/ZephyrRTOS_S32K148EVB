#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/zbus/zbus.h>

/*DONANIM TANIMLARI*/
static const struct device *adc_dev= DEVICE_DT_GET(DT_NODELABEL(adc0));
static const struct pwm_dt_spec pwm_led= PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));
#define POT_CHANNEL 28

/*zbus ile taşınacak verinin kalıbını ayarlıyoruz*/
struct sensor_verisi{
    uint16_t pot_degeri
};


ZBUS_SUBSCRIBER_DEFINE(pwm_dinleyicisi,4); //aboneyi sisteme tanımlar adı ve büyüklüğü
/*Zbus kanalı tanımı.       
ZBUS_OBSERVERS=Bu makro, kanalın gözlemcileri listelediğini gösterir. 
ZBUS_MSG_INIT=Bu makro, mesaj talimatını veya birliğini başlatmak için değerleri aktararak bir mesajı başlatır.*/
ZBUS_CHAN_DEFINE(sensor_kanali,struct sensor_verisi,NULL,NULL,ZBUS_OBSERVERS(pwm_dinleyicisi),ZBUS_MSG_INIT(.pot_degeri=0));

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
    struct sensor_verisi giden_paket;
    while(1)
    {
        if(adc_read(adc_dev,&sequence)==0)
        {
            giden_paket.pot_degeri = okunan_deger;
            /*Bu rutin bir kanala bir mesaj yayınlar.*/
            zbus_chan_pub(&sensor_kanali, &giden_paket, K_MSEC(10));
        }

        k_msleep(50);
    }
}

void pwm_kontrol_thread(void *arg1, void *arg2, void *arg3)
{
    const struct zbus_channel *aktif_kanal;
    struct sensor_verisi gelen_paket;
    uint16_t gelen_deger;
    uint32_t periyot = 1000000;

    while (1)
    {   /*Bu rutin aboneyi bir bildirim beklemeye getirir. Bildirim bir kanal referansı olarak geliyor.*/
        if(zbus_sub_wait(&pwm_dinleyicisi, &aktif_kanal, K_FOREVER) == 0)
        {
            if (aktif_kanal == &sensor_kanali)
            {   /*Bu rutin bir kanaldan bir mesaj okur.*/
                zbus_chan_read(&sensor_kanali, &gelen_paket, K_NO_WAIT);
                
                /* Veriyi işleyip motoru (PWM) sürüyoruz */
                uint32_t pulse = ((uint64_t)gelen_paket.pot_degeri * periyot) / 4095;
                pwm_set(pwm_led.dev, pwm_led.channel, periyot, pulse, pwm_led.flags);
            }   
            
        }
    }
    
}

K_THREAD_DEFINE(adc_tid, 1024, adc_okuma_thread, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(pwm_tid, 1024, pwm_kontrol_thread, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    while (1) {
        
    }
    return 0;
}