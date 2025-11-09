/*
    Erstellt 09.11.25 
    
    ESP32 mit 240x320 Pixel st7789 Display am SPI Bus 
    Grafikintervace lvgl 9.2 (Declariert in der idf_comonenten.yml) neuere Versionen Funktionieren derzeit nicht mit Platformio
    ESP-IDF Verison v5.2.1 deklariert in platformio.ini (platform = espressif32@6.6.0) neuere Verison funktionieren aktuel nicht 

    Konfiguration der lvgl über build_flags = -D LV_CONF_PATH="${PROJECT_DIR}/include/lv_conf.h" in PLatformio.ini 
    Die lv_config.h liegt im include Ordner. 
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

#include "MyLVGLDisplST7789.h"


void app_main() 
{

    My_LVGL_Disp_main_Init();
    
}
