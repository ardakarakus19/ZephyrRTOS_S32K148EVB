#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

static const struct gpio_dt_spec led=GPIO_DT_SPEC_GET(DT_ALIAS(led0),gpios);

void led_timer_tetiklecisi(struct k_timer *timer_id)
{
    gpio_pin_toggle_dt(&led);
    printk("Timer Tetiklendi! LED Durumu Degisti.\n");
}

K_TIMER_DEFINE(heartbeat_timer,led_timer_tetiklecisi,NULL);/*timerı ayarlou-yoruz 1fonksiyon adı 2alarm çaldığında nereye gidecek 3 alarm iptal olursa ne yapacak */

int main(void)
{
    gpio_pin_configure_dt(&led,GPIO_OUTPUT_ACTIVE);//ledi output olarak ayarlıyoruz
    
    k_timer_start(&heartbeat_timer, K_SECONDS(2), K_SECONDS(2));/*timerı ayarlıypruz hangisi başlatılcak ne kadar bekletilcek ve kaç sn debir tekrra edicek*/

    while (1) {
        k_msleep(10000); /* 10 saniyede bir uyanıp geri uyur, hiçbir şey yapmaz */
    }
    
    return 0;
}