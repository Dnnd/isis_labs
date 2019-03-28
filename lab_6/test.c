#include <stdio.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <zconf.h>
#include <string.h>
#include <stdlib.h>

#define LEDCMD_RESET_STATE _IO(HELLO_MAJOR, 1)                 /* погасить все индикаторы */
#define LEDCMD_GET_LED_STATE _IOWR(HELLO_MAJOR, 3,  led_t *) /* вернуть значение индикатора с указанным номером */
#define LEDCMD_SET_LED_STATE _IOW(HELLO_MAJOR, 4, led_t *) /* установить значение индикатора с указанным номером */

/* Структура для передачи состояния одного светодиода */
#define  HELLO_MAJOR 250

typedef struct {
    int pin;
    unsigned char value;
} led_t;


int set_led_state(int pin, char val) {
    int fd = open("/dev/hello", O_RDWR);
    led_t led = {.pin = pin, .value = val};
    int err = ioctl(fd, LEDCMD_SET_LED_STATE, &led); /* Получаем текущее состояние */
    if (err) {
        printf("error on set\n");
        close(fd);
        return err;
    }
    printf("pin: %d, val: %d\n", led.pin, led.value);
    close(fd);
    return 0;
}

int get_led_state(int pin) {
    led_t led = {.pin = pin};
    int fd = open("/dev/hello", O_RDWR);
    if (fd == -1) {
        return -1;
    }
    int err = ioctl(fd, LEDCMD_GET_LED_STATE, &led); /* Получаем текущее состояние */
    if (err) {
        printf("error on get\n");
        close(fd);
        return err;
    }
    printf("pin: %d, val: %d\n", led.pin, led.value);
    close(fd);
    return 0;
}

int reset_leds() {
    int fd = open("/dev/hello", O_RDWR);
    int err = ioctl(fd, LEDCMD_RESET_STATE);
    if (err) {
        printf("error on get\n");
        close(fd);
        return err;
    }
    close(fd);
    return 0;
}

int get_all_leds() {
    for (int i = 0; i < 8; ++i) {
        int err = get_led_state(i);
        if (err) {
            return err;
        }
    }
    return 0;
}

int run_commands(int argc, char **argv) {
    if (argc == 2) {
        if (!strcmp(argv[1], "reset")) {
            return reset_leds();
        } else if (!strcmp(argv[1], "ledstate")) {
            return get_all_leds();
        }
    } else if (argc == 3) {
        if (!strcmp(argv[1], "on")) {
            char *end;
            int pin = (int) strtol(argv[2], &end, 10);
            set_led_state(pin, 1);
        } else if (!strcmp(argv[1], "off")) {
            char *end;
            int pin = (int) strtol(argv[2], &end, 10);
            set_led_state(pin, 0);
        } else if (!strcmp(argv[1], "ledstate")) {
            char *end;
            int pin = (int) strtol(argv[2], &end, 10);
            get_led_state(pin);
        }
    }
    return -1;
}

int main(int argc, char **argv) {
    return run_commands(argc, argv);
}
