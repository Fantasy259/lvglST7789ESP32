/*
    Erstellt 09.11.25 
    
    ESP32 mit 240x320 Pixel st7789 Display am SPI Bus 
    Grafikintervace lvgl 9.2 (Declariert in der idf_comonenten.yml) neuere Versionen Funktionieren derzeit nicht mit Platformio
    ESP-IDF Verison v5.2.1 deklariert in platformio.ini (platform = espressif32@6.6.0) neuere Verison funktionieren aktuel nicht 

    Konfiguration der lvgl über build_flags = -D LV_CONF_PATH="${PROJECT_DIR}/include/lv_conf.h" in PLatformio.ini 
    Die lv_config.h liegt im include Ordner. 
*/

#include "MyLVGLDisplST7789.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"

#include "lvgl.h"
#include "./src/drivers/display/st7789/lv_st7789.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

// ****  Einbinden der Dateien die mit EZZ Studio erstellt wurden https://www.envox.eu/studio/studio-introduction/
// Im projekt sollte der ordner für lvgl nur mit lvgl angeben werden nicht mit lvgl/lvgl
#include "ui/ui.h" 

#define LCD_H_RES       240
#define LCD_V_RES       320
#define BUS_SPI1_POLL_TIMEOUT 0x1000U

#define TFT_MOSI    25 //23
#define TFT_SCLK    26 // 18
#define TFT_CS      33 // 0
#define TFT_DC      32 // 2
#define TFT_RST     16 // 4

static const char *TAG = "MyMain";

// Using SPI2 in the example
#define LCD_HOST  SPI2_HOST
static spi_device_handle_t spi;

lv_display_t *lcd_disp;

// Declaration Function 
static void My_LVGL_Disp_start_lvgl_tick(void);     // Starten der Zeit lvgl 
static void My_LVGL_Disp_lv_tick_cb(void *arg);     // lvgl Tick Zeit 
static int32_t My_LVGL_Disp_lcd_io_SPI_init(void);  // SPI einrichten 
static void My_LVGL_Disp_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size);  // lvgl funktion zum senden von Befehle 
static void My_LVGL_Disp_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size); // lvgl Funktion zum senden von Farben 
void My_LVGL_Disp_Time_Handler(void *arg);

void My_LVGL_Disp_main_Init()
{
       ESP_LOGI(TAG , "-Start -");

    /* Initialize LVGL */
    lv_init();

    /* Initialize LCD SPI interface */
    My_LVGL_Disp_lcd_io_SPI_init();

    // Zeitgeber für lvgl. Ohne die funktion bleibt LVGL stehen und es wird ein Watchdog ausgelöst vom LVGL 
    My_LVGL_Disp_start_lvgl_tick(); 
    
    /* Create the LVGL display object and the LCD display driver */
    lcd_disp = lv_st7789_create(LCD_H_RES, LCD_V_RES, LV_LCD_FLAG_NONE, My_LVGL_Disp_lcd_send_cmd, My_LVGL_Disp_lcd_send_color);
    lv_display_set_rotation(lcd_disp, LV_DISPLAY_ROTATION_0);
    lv_st7789_set_invert(lcd_disp, true);
    
    /* Allocate draw buffers on the heap. In this example we use two partial buffers of 1/10th size of the screen */
    lv_color_t * buf1 = NULL;
    lv_color_t * buf2 = NULL;

    uint32_t buf_size = LCD_H_RES * LCD_V_RES / 10 * lv_color_format_get_size(lv_display_get_color_format(lcd_disp));

    buf1 = lv_malloc(buf_size);
    if(buf1 == NULL) {
            ESP_LOGI(TAG, "display draw buffer malloc failed");
            return;
    }

    buf2 = lv_malloc(buf_size);
    if(buf2 == NULL) {
            ESP_LOGI(TAG,"display buffer malloc failed");
            lv_free(buf1);
            return;
    }
    lv_display_set_buffers(lcd_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

   //ui_init(lcd_disp);
   ui_init();           // Aufruf der mit EEZ Studio erzeugten Seite. 

   xTaskCreate(My_LVGL_Disp_Time_Handler, "My_LVGL_Disp_Time_Handler", 4096, NULL, 5 ,NULL);  // Thread einschalten 

}

// ***************************************************************************************
// ***** worker für die lvgl 
// ***************************************************************************************
void My_LVGL_Disp_Time_Handler(void *arg)
{
    while(true)
    {
                    /* The task running lv_timer_handler should have lower priority than that running `lv_tick_inc` */
            lv_timer_handler();
            /* raise the task priority of LVGL and/or reduce the handler period can improve the performance */
            vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ********************************************************************
// **** Zeitgeber für die lvgl muss gestartet werden vor befor der Display erzeugt wird. 
// **** Wird im 1ms Takt auferufen 
// ********************************************************************
static void My_LVGL_Disp_lv_tick_cb(void *arg) {
    lv_tick_inc(1);  // +1 ms
}

// ********************************************************************
// **** Startet den Zeitgeber für lvgl 
// **** Wird im einrichtenbereich aufgerufen 
// ********************************************************************
static void My_LVGL_Disp_start_lvgl_tick(void)
{
    const esp_timer_create_args_t tcfg = {
        .callback = &My_LVGL_Disp_lv_tick_cb,
        .name     = "lvgl_tick"
    };
    static esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tcfg, &tick_timer));
    // alle 1000 µs (1 ms)
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, 1000));
}


// ***************************************************************************
// ****** Einrichten des SPI schnitstelle für die Datenübertragung an dens Display 
// ***************************************************************************
static int32_t My_LVGL_Disp_lcd_io_SPI_init(void)
{
    // GPIOs funktion einstellen  
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << TFT_CS) | (1ULL << TFT_RST) | (1ULL << TFT_DC) ,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Reset Sequenz für den Dsiaply 
    gpio_set_level(TFT_RST, 0);         // Reset aktiv
    vTaskDelay(pdMS_TO_TICKS(20));      // mindestens 10µs, wir geben 20ms
    gpio_set_level(TFT_RST, 1);         // Reset aus
    vTaskDelay(pdMS_TO_TICKS(120));     // warten bis Controller bereit

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = TFT_SCLK,
        .mosi_io_num = TFT_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 24 * 1000 * 1000,             // Speed 24 MHZ
        .mode = 0,
        .spics_io_num = TFT_CS,
        .queue_size = 10,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

        return true;
}


// *************************************************************************************
// ****** Route für das übertragen von Befehle vom lvgl an den Display 
// ****** Wird aus dem lvgl selbst aufgerufn 
// *************************************************************************************
static void My_LVGL_Disp_lcd_send_cmd(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, const uint8_t *param, size_t param_size)
{
  //   ESP_LOGI("ST7735", "send_CMD cmd=0x%02X, size cmd =%d , param = 0x%02x, size Param %d", cmd ? cmd[0] : 0, (int) cmd_size, param ? param[0] : 0, (int)param_size);
    esp_err_t ret;

    // 1. Command senden (DC = 0)
    gpio_set_level(TFT_DC, 0);
    spi_transaction_t t_cmd = {
        .length = cmd_size * 8,
        .tx_buffer = cmd,
    };
    ret = spi_device_polling_transmit(spi, &t_cmd);
  
    if (ret != ESP_OK) return;

    // 2. Parameterdaten senden (falls vorhanden, DC = 1)
    if (param != NULL && param_size > 0) {
        gpio_set_level(TFT_DC, 1);
        spi_transaction_t t_data = {
            .length = param_size * 8,
            .tx_buffer = param,
        };
        ret = spi_device_polling_transmit(spi, &t_data);
        if (ret != ESP_OK) return;
    }

    return;
}

// ***********************************************************************************************
// **** Routine für das Übertragen der Bild Informationen an den Display Farbformat RGB565(16bit) 
// **** Wird von lvgl selber aufgerufen 
// ************************************************************************************************
static void My_LVGL_Disp_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size)
{
   esp_err_t ret;
    // 1. Command schicken (z. B. 0x2C)
    if (cmd != NULL && cmd_size > 0) {
        gpio_set_level(TFT_DC, 0);
        spi_transaction_t t_cmd = {
            .length = cmd_size * 8,
            .tx_buffer = cmd,
        };
        ret = spi_device_polling_transmit(spi, &t_cmd);
        if (ret != ESP_OK) return;
    }

    // 2. Pixel-Daten (param)
    if (param != NULL && param_size > 0) {
        gpio_set_level(TFT_DC, 1);
        spi_transaction_t t_data = {
            .length = param_size * 8,
            .tx_buffer = param,
        };
        ret = spi_device_polling_transmit(spi, &t_data);
        if (ret != ESP_OK) return;
    }

    lv_display_flush_ready(disp);
    return; // Erfolg
}


// Eigne erzeugen eine Display mit animation 
/*
static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

void ui_init(lv_display_t *disp)
{
        lv_obj_t *obj;

        // set screen background to white 
        lv_obj_t *scr = lv_screen_active();
        lv_obj_set_style_bg_color(scr, lv_color_make(0, 0, 0), 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_100, 0);

        // create label 
        obj = lv_label_create(scr);
 //       lv_obj_set_align(obj, LV_ALIGN_CENTER);
        lv_obj_set_height(obj, LV_SIZE_CONTENT);
        lv_obj_set_width(obj, LV_SIZE_CONTENT);

        lv_obj_set_style_text_font(obj, &lv_font_montserrat_18, 0);
//        lv_obj_set_style_text_color(obj, lv_color_black(), 0);
        lv_obj_set_style_text_color(obj, lv_color_make(0, 0, 255), 0);
        lv_label_set_text(obj, "Hello");
        lv_obj_set_pos(obj, 20,20);

        lv_anim_t MyAni;
        lv_anim_init(&MyAni);
                //Set the "animator" function
        lv_anim_set_exec_cb(&MyAni, (lv_anim_exec_xcb_t) anim_x_cb);

        //Set target of the animation
        lv_anim_set_var(&MyAni, obj);

        //Length of the animation [ms]
        lv_anim_set_duration(&MyAni, 5000);

        //Set start and end values. E.g. 0, 150
        lv_anim_set_values(&MyAni, 20, 220);

        lv_anim_set_path_cb(&MyAni, lv_anim_path_overshoot);

        lv_anim_set_repeat_count(&MyAni, LV_ANIM_REPEAT_INFINITE);

        lv_anim_start(&MyAni);     

}
*/


