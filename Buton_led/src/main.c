#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
#define SW0_NODE  DT_ALIAS(sw0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

int main(void)
{
    int ret;

    if (!device_is_ready(led.port) || !device_is_ready(button.port)) {
        return 0;
    }

    // LED'i çıkış olarak ayarla ve başlangıçta söndür
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        return 0;
    }

    // Butonu giriş olarak ayarla
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret < 0) {
        return 0;   
    }

    while (1) {
        // Buton durumunu oku (Basılı: 1, Serbest: 0)
        int val = gpio_pin_get_dt(&button);
        gpio_pin_set_dt(&led,0);

        if (val == 0) {
            
            gpio_pin_set_dt(&led, 1);
        }
        else
        {
            gpio_pin_set_dt(&led,0);
        }
        // İşlemci yükünü azaltmak ve ark (debounce) önlemek için kısa gecikme
        k_msleep(10);
    }
    return 0;
}