
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
