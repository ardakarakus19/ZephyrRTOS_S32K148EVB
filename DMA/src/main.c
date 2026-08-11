#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

static const struct device *adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));

#define POT_CHANNEL 28
uint16_t adc_val;

/* --- 1. DMA CALLBACK --- */
static enum adc_action adc_dma_bitti_callback(const struct device *dev,
                                              const struct adc_sequence *sequence,
                                              uint16_t sampling_index)
{
    /* FIRTINAYI DURDURAN YER: 
       REPEAT yerine FINISH diyoruz. İşini bitir ve işlemciyi rahat bırak. */
    return ADC_ACTION_FINISH;
}

/* --- 2. DMA AYARLARI --- */
struct adc_sequence_options dma_ayarlari = {
    .callback = adc_dma_bitti_callback,
    .user_data = NULL,
};

struct adc_sequence sequence = {
    .options     = &dma_ayarlari,
    .channels    = BIT(POT_CHANNEL),
    .buffer      = &adc_val,
    .buffer_size = sizeof(adc_val),
    .resolution  = 12,
};

/* --- 3. ANA FONKSİYON --- */
int main(void)
{
    /* Artık sistem kilitlenmeyeceği için bu yazıyı ekranda görebileceğiz */
    printk("Kontrollü Asenkron ADC Mimarisi Başlıyor...\n");

    if (!device_is_ready(adc_dev)) {
        printk("ADC donanımı hazır değil!\n");
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
        adc_channel_setup(adc_dev, &channel_cfg);
    }
     
    /* Ana döngü */
    while (1) {
        
            adc_read_async(adc_dev, &sequence, NULL);
        
        printk("Guncel Deger: %d\n", adc_val);
        
        
        /* 3. İşlemciyi 1 saniye uyut. Bu sırada DMA arka planda işini halleder */
        k_msleep(1000); 
    }
    
    return 0;
}