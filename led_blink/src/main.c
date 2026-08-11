#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;

    if (!device_is_ready(led.port)) {
        return 0;
    }

    // Pini çıkış olarak ayarla ve doğrudan aktif (yanık) duruma getir
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        return 0;
    }

    // Döngünün içinde herhangi bir işlem yapmaya gerek yok, 
    // LED ilk anda açıldığı gibi sürekli yanmaya devam eder.
    while (1) {
        k_msleep(1000);
    }

    return 0;
}