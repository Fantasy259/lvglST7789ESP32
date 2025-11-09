#ifndef LV_CONF_H
#define LV_CONF_H

/* Erzwingt einfaches Include */
#define LV_CONF_INCLUDE_SIMPLE

/*******************
 * LVGL Version
 *******************/
#define LV_VERSION_MAJOR   9
#define LV_VERSION_MINOR   2
#define LV_VERSION_PATCH   0

/*******************
 * Display Auflösung
 *******************/
#define LV_HOR_RES_MAX    240
#define LV_VER_RES_MAX    320

/*******************
 * Farbeinstellungen
 *******************/
#define LV_COLOR_DEPTH    16
#define LV_COLOR_16_SWAP  1   /* Für ST7789 empfehlenswert */
#define LV_COLOR_SCREEN_TRANSP 0

/*******************
 * Schriftarten
 *******************/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/*******************
 * Speicherverwaltung
 *******************/
#define LV_MEM_CUSTOM     0
#define LV_MEM_SIZE       (64U * 1024U)   /* 64KB für LVGL — stabil für Animations + UI */
#define LV_MEM_ADR        0

#define LV_USE_ANIMATION 1

/*******************
 * Tick / Time
 *******************/
#define LV_TICK_CUSTOM 0

/*******************
 * Logging
 *******************/
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/*******************
 * Widgets (nur Basis)
 *******************/
#define LV_USE_BTN     1
#define LV_USE_LABEL   1
#define LV_USE_IMG     1
#define LV_USE_SLIDER  1
#define LV_USE_BAR     1

/*******************
 * Keine Touch Eingabe aktivieren
 *******************/
#define LV_USE_INDev 1
#define LV_INDEV_DEF_READ_PERIOD 30
/* Du nutzt **keinen Touch** → später keine `lv_indev_drv_register()` nötig */

#endif /* LV_CONF_H */
