#include <string.h>

#include "screens.h"
#include "images.h"
#include "fonts.h"
#include "actions.h"
#include "vars.h"
#include "styles.h"
#include "ui.h"

#include "esp_log.h"

#include <string.h>

objects_t objects;
lv_obj_t *tick_value_change_obj;
uint32_t active_theme_index = 0;




static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

void create_screen_main() {
    lv_obj_t *obj = lv_obj_create(0);
    objects.main = obj;
    lv_obj_set_pos(obj, 0, 0);
    lv_obj_set_size(obj, 240, 320);

    {
        lv_obj_t *Aniobj;

        lv_obj_t *parent_obj = obj;
        {
            lv_obj_t *obj = lv_label_create(parent_obj);
            lv_obj_set_pos(obj, 34, 24);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_set_style_align(obj, LV_ALIGN_TOP_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_font(obj, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(obj, "ESP-Hello World ");
        }
        {
            lv_obj_t *obj = lv_image_create(parent_obj);
            lv_obj_set_pos(obj, 56, 96);
            lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_image_set_src(obj, &img_bild);
        }
        {
            lv_obj_t *obj = lv_led_create(parent_obj);
            objects.obj0 = obj;
            Aniobj = obj;
            lv_obj_set_pos(obj, 20, 242);
            lv_obj_set_size(obj, 16, 16);
            lv_led_set_color(obj, lv_color_hex(0xffff44db));
            lv_led_set_brightness(obj, 255);
        }

        lv_anim_t MyAni;
        lv_anim_init(&MyAni);
        
        lv_anim_set_exec_cb(&MyAni, (lv_anim_exec_xcb_t) anim_x_cb);  // Animation funktcion
        lv_anim_set_var(&MyAni, Aniobj);   
        lv_anim_set_duration(&MyAni, 3000);                         // duration timer for animation forword 
        lv_anim_set_playback_time(&MyAni, 3000);                     // duration time animation back 
        lv_anim_set_values(&MyAni, 20, 200);                        // Values vor animation will set in Funktion 
        lv_anim_set_path_cb(&MyAni, lv_anim_path_ease_in_out);
        lv_anim_set_repeat_count(&MyAni, LV_ANIM_REPEAT_INFINITE);
        lv_anim_start(&MyAni); 
    }
    
    tick_screen_main();
}

void tick_screen_main() {
}



typedef void (*tick_screen_func_t)();
tick_screen_func_t tick_screen_funcs[] = {
    tick_screen_main,
};
void tick_screen(int screen_index) {
    tick_screen_funcs[screen_index]();
}
void tick_screen_by_id(enum ScreensEnum screenId) {
    tick_screen_funcs[screenId - 1]();
}

void create_screens() {
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    
    create_screen_main();
}
