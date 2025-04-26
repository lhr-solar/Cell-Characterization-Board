// main.c 
// For use in the Longhorn Racing Solar Cell Characterization Board
// By Dylan and Devin Kretschman
// Created 2/15/2025

#include "esp32s3/rom/gpio.h" // esp32-s3 gpio functions for standard output
#include "esp32s3/rom/uart.h" // esp32-s3 uart functions 
#include "esp_adc/adc_continuous.h"

#include "hal/spi_types.h"

#include "freertos/FreeRTOS.h"




// void app_main2() {
//     UART_Init(0,115200);
//     send_packet("balls",6);

// }