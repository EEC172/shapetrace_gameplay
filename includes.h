#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Simplelink includes
#include "simplelink.h"

//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "spi.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_apps_rcm.h"
#include "glcdfont.h"
#include "prcm.h"
#include "gpio.h"
#include "systick.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"

//Common interface includes
//#include "pinmux.h"
#include "pin_mux_config.h"
#include "gpio_if.h"
#include "common.h"
#include "uart_if.h"
#include "i2c_if.h"

#endif // includes.h
