#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/can.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(can_uygulamasi, LOG_LEVEL_INF);

/* Device tree'den seçili CAN cihazını al */
const struct device *const can_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_canbus));

int main(void)
{
    if (!device_is_ready(can_dev)) {
        LOG_ERR("CAN cihazi hazir degil!");
        return -ENODEV;
    }

    /* CAN modülünü normal operasyon moduna al */
    can_set_mode(can_dev, CAN_MODE_NORMAL);
    can_start(can_dev);

    /* Gonderilecek standart CAN çerçevesi */
    struct can_frame frame = {
        .id = 0x123,
        .flags = 0,         /* Standart 11-bit ID */
        .dlc = 8,           /* 8 byte veri uzunluğu */
        .data = {1, 2, 3, 4, 5, 6, 7, 8}
    };

    LOG_INF("CAN mesaji gonderme dongusu basliyor...");

    while (1) {
        int ret = can_send(can_dev, &frame, K_MSEC(100), NULL, NULL);
        if (ret != 0) {
            LOG_ERR("Mesaj gonderilemedi, Hata kodu: %d", ret);
        } else {
            LOG_INF("0x123 ID'li mesaj basariyla gonderildi!");
        }
        
        k_sleep(K_SECONDS(1));
    }
    return 0;
}