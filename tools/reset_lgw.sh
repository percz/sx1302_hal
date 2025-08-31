#!/bin/sh

# SX1302 CoreCell GPIO reset script supporting both legacy /sys/class/gpio and modern gpioset

SX1302_RESET_PIN=25     # SX1302 reset
SX1302_POWER_EN_PIN=8  # SX1302 power enable

GPIO_CHIP=gpiochip0
RESET_GPIO="$SX1302_RESET_PIN"
POWER_EN_GPIO=$SX1302_POWER_EN_PIN
POWER_EN_LOGIC=1
GPIOSET="gpioset -m time -u 100000 ${GPIO_CHIP}"

WAIT_GPIO() {
    sleep 0.1
}

use_gpioset() {
    command -v gpioset >/dev/null 2>&1
}

init_sysfs() {
    echo "$SX1302_RESET_PIN" > /sys/class/gpio/export; WAIT_GPIO
    echo "out" > /sys/class/gpio/gpio$SX1302_RESET_PIN/direction; WAIT_GPIO
}

reset_sysfs() {
    echo "CoreCell reset through GPIO$SX1302_RESET_PIN..."

    echo "1" > /sys/class/gpio/gpio$SX1302_POWER_EN_PIN/value; WAIT_GPIO
    echo "1" > /sys/class/gpio/gpio$SX1302_RESET_PIN/value; WAIT_GPIO
    echo "0" > /sys/class/gpio/gpio$SX1302_RESET_PIN/value; WAIT_GPIO
}

term_sysfs() {
    if [ -d /sys/class/gpio/gpio$SX1302_RESET_PIN ]; then
        echo "$SX1302_RESET_PIN" > /sys/class/gpio/unexport; WAIT_GPIO
    fi
}

enable_gpioset() {
    if [ "$POWER_EN_GPIO" -ne 0 ]; then
        echo "SET ${GPIO_CHIP}:1"
        ${GPIOSET} ${POWER_EN_GPIO}=${POWER_EN_LOGIC} 2>/dev/null
    fi
}

disable_gpioset() {
    if [ "$POWER_EN_GPIO" -ne 0 ]; then
        echo "SET ${GPIO_CHIP}:0"
        ${GPIOSET} ${POWER_EN_GPIO}=0 2>/dev/null
    fi
}

reset_gpioset() {
    for GPIO in $(echo $RESET_GPIO | tr ',' ' '); do
        if [ "$GPIO" -ne 0 ]; then
            echo "SET ${GPIO_CHIP}:${GPIO}"
            ${GPIOSET} "${GPIO}"=0 2>/dev/null
            ${GPIOSET} "${GPIO}"=1 2>/dev/null
            ${GPIOSET} "${GPIO}"=0 2>/dev/null
        fi
    done
}

case "$1" in
    start)
        if use_gpioset; then
            disable_gpioset # just in case
            enable_gpioset
            reset_gpioset
        else
            term_sysfs # just in case
            init_sysfs
            reset_sysfs
        fi
        ;;
    stop)
        if use_gpioset; then
            reset_gpioset
            disable_gpioset
        else
            reset_sysfs
            term_sysfs
        fi
        ;;
    *)
        echo "Usage: $0 {start|stop}"
        exit 1
        ;;
esac

exit 0