#include <stdio.h>
#include <gtk/gtk.h>
//#include <gtk/gtkprogressbar.h>
#include "usb.h"

#define TEST_BTN_AND_OLED_ONLY 
#define TEST_FULL_TEST

//#define DEVELOP_TEST
#define MUTI_THREAD_SUPPORT

#define threads_init()
#define threads_enter()
#define threads_leave()

#define BTN_TEST_TIMEOUT 5 
#define RESULT_PASS_STR "<span foreground='black'>成功</span>"
#define RESULT_FAILED_STR "<span foreground='black'>失败</span>"
#define RESULT_RESET_STR "<span foreground='black'> </span>"
#define RESULT_TEST_KEY_OK_STR "<span foreground='red'>请按[左键][Left]</span>"
#define RESULT_TEST_KEY_CANCEL_STR "<span foreground='red'>请按[右键][Right]</span>"

#define RESULT_TEST_OLED_LIGHT_STR "<span foreground='red'>屏[全亮]显示正常(PASS/FAIL) ?</span>"
#define RESULT_TEST_OLED_DARK_STR "<span foreground='red'>屏[全暗]显示正常(PASS/FAIL) ?</span>"

#define RESULT_TEST_FW_EARSE_START_STR "<span foreground='red'>正在执行固件擦除..</span>"
#define RESULT_TEST_FW_EARSE_EXIT_STR "<span foreground='red'>固件擦除成功!</span>"
#define RESULT_TEST_TIMEOUT_STR "测试超时!"

#define START_BUTTON_GO_STR "开始测试"
#define START_BUTTON_TESTING_STR "测试中.."

#define TEST_CMD_SEND(x) \
    do { \
        if (usb_write(x) < 0) { \ 
            printf("[%s:%s] send cmd[%s] failed.\n", __FILE__, __func__, x); \
        } \
    } while(0)


static int test_item_id = 0;

static GtkLabel *label_btn_left_result = NULL;
static GtkLabel *label_btn_right_result = NULL;
static GtkLabel *label_oled_full_light = NULL;
static GtkLabel *label_oled_full_dark = NULL;
static GtkLabel *label_firmware_erase = NULL;

static GtkLabel *label_btn_left_item = NULL;
static GtkLabel *label_btn_right_item = NULL;
static GtkLabel *label_oled_full_lg_item = NULL;
static GtkLabel *label_oled_full_dk_item = NULL;
static GtkLabel *label_firmware_erase_item = NULL;
static GtkWidget *window;

static GtkButton *button_start;


static GdkColor red = {0, 0xffff, 0, 0};
static GdkColor green = {0, 0, 0xffff, 0};
static GdkColor blue = {0, 0, 0, 0xffff};
static GdkColor yellow = {0, 0xffff, 0xffff, 0};
static GdkColor cyan = {0, 0, 0xffff, 0xffff};
static GdkColor black = {0, 0, 0, 0};


static GdkColor grey51;//grid
static GdkColor grey81;//item
static GdkColor LightSkyBlue2;

static GThread *g_thread = NULL;

static guint mCheckScreenTimer = 0;
static guint mTimer = 0;

static int is_in_test = 0;
static int time_out = 0;

static int hw_btn_left_test_ok = 0;
static int hw_btn_right_test_ok = 0;

static int screen_light_test_ok = 0;
static int screen_dark_test_ok = 0;

static GtkWidget *check_oled_dialog = NULL;
static GtkWidget *frimware_earse_dialog = NULL;

typedef struct USB_RESP_T {
    int resp;
    int result;
};

static struct USB_RESP_T mUsbRespT;

static void update_btn_test_show_info(int id, int result);
static void usb_msg_callback(int id, int result);

static void reset_test_state(void)
{
    printf("[%s:%s]\n", __FILE__, __func__);

    if (g_thread) {
        TEST_CMD_SEND("usb-disconnect");
        g_thread = NULL;
    }

    hw_btn_left_test_ok = 0;
    hw_btn_right_test_ok = 0;
    is_in_test = 0;
    mTimer = 0;
    mCheckScreenTimer = 0;
    time_out = 0;

    screen_light_test_ok = 0;
    screen_dark_test_ok = 0;

    gtk_button_set_label(GTK_BUTTON(button_start), START_BUTTON_GO_STR);
    gtk_widget_set_sensitive(GTK_BUTTON(button_start), TRUE);
}

static void test_item_ui_reset(void) 
{
    update_btn_test_show_info(TEST_HW_BTN_LEFT, RESULT_RESET);
    update_btn_test_show_info(TEST_HW_BTN_RIGHT, RESULT_RESET);

#ifdef TEST_BTN_AND_OLED_ONLY
    update_btn_test_show_info(TEST_OLED_LIGHT, RESULT_RESET);
    update_btn_test_show_info(TEST_OLED_DARK, RESULT_RESET);
#endif

#ifdef TEST_FULL_TEST
    update_btn_test_show_info(TEST_FIRMWARE_ERASE, RESULT_RESET);
#endif
}

static gboolean test_time_out(gpointer* data)
{
    printf("[%s:%s] \n", __FILE__, __func__);
	if (mTimer == 0) 
		return FALSE;

    threads_enter();
    update_btn_test_show_info(*((int *)data), RESULT_TEST_TIME_OUT);
	reset_test_state();
	threads_leave();

    if (mTimer)
		mTimer = 0;
	
	return FALSE;
}

static void callback_for_hw_btn_test(GtkWidget *wid, GtkWidget *win)
{
    if (!is_in_test) {
        if ((g_thread = register_usb_callback(usb_msg_callback)) == NULL) {
            printf("[%s:%s] g_thread create failed!\n", __FILE__, __func__);

            GtkWidget *dialog = gtk_message_dialog_new_with_markup(
                    GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Can not connect device USB !");
            gtk_window_set_position(GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }

        test_item_ui_reset();

        gtk_button_set_label(GTK_BUTTON(button_start), START_BUTTON_TESTING_STR);
        gtk_widget_set_sensitive(GTK_BUTTON(button_start), FALSE);

        // test key 1
        TEST_CMD_SEND("#test_hw_key#");

        test_item_id = TEST_HW_BTN_LEFT;
        update_btn_test_show_info(test_item_id, RESULT_TEST_KEY_OK);		
		mTimer = g_timeout_add(1000*5, (GSourceFunc)test_time_out, &test_item_id);
		is_in_test = 1;
    }
}

static void g_UI_thread_worker(int resp_id, int result)
{
    if (test_item_id != resp_id && resp_id != TEST_RESET) {
        printf("[%s:%s] %d,%d, resp_id not current test item return\n", __FILE__, __func__);
        return ;
    }

    if (is_in_test) {
        threads_enter();
        update_btn_test_show_info(resp_id, result);
		
        switch(resp_id) {
            case TEST_HW_BTN_LEFT:
				g_source_remove(mTimer);
                if (result == RESULT_PASS) {
                    printf("[%s:%s] %d,%d, TEST_HW_BTN_LEFT test pass !\n", __FILE__, __func__);
                    hw_btn_left_test_ok = 1;

                    // test key 2
                    TEST_CMD_SEND("#test_hw_key#");
                    test_item_id++;
                    update_btn_test_show_info(test_item_id, RESULT_TEST_KEY_CANCEL);
					mTimer = g_timeout_add(1000*5, (GSourceFunc)test_time_out, &test_item_id);
                }
                break;

            case TEST_HW_BTN_RIGHT:
				g_source_remove(mTimer);
                if (result == RESULT_PASS) {
                    printf("[%s:%s] %d,%d, TEST_HW_BTN_RIGHT test pass !\n", __FILE__, __func__);
                    hw_btn_right_test_ok = 1;

#ifdef TEST_BTN_AND_OLED_ONLY
                    // test screen light
                    TEST_CMD_SEND("#test_screen_light_display#");
                    test_item_id++;
                    update_btn_test_show_info(test_item_id, RESULT_TEST_OLED_LIGHT);
					mTimer = g_timeout_add(1000*5, (GSourceFunc)test_time_out, &test_item_id);
#else
                    reset_test_state();
#endif
                }
                break;

            case TEST_OLED_LIGHT:
				g_source_remove(mTimer);
                if (result == RESULT_PASS) {
                    printf("[%s:%s] %d,%d, TEST_OLED_LIGHT test pass !\n", __FILE__, __func__);
                    screen_light_test_ok = 1;

                    // test screen dark
                    TEST_CMD_SEND("#test_screen_dark_display#");
                    test_item_id++;
                    update_btn_test_show_info(test_item_id, RESULT_TEST_OLED_DARK);
					mTimer = g_timeout_add(1000*5, (GSourceFunc)test_time_out, &test_item_id);
                }
                else {
                    reset_test_state();
                }
                break;

            case TEST_OLED_DARK:
				g_source_remove(mTimer);
                if (result == RESULT_PASS) {
                    printf("[%s:%s] %d,%d, TEST_OLED_DARK test pass !\n", __FILE__, __func__);
                    screen_dark_test_ok = 1;

#ifdef TEST_FULL_TEST
                    // test firmware erase.
#ifndef DEVELOP_TEST
                    TEST_CMD_SEND("#Erase fw#");
#else
                    TEST_CMD_SEND("1Erase fw");
#endif
                    test_item_id++;
                    update_btn_test_show_info(test_item_id, RESULT_TEST_FW_EARSE_START);
					mTimer = g_timeout_add(1000*5, (GSourceFunc)test_time_out, &test_item_id);
#else
                    reset_test_state();
#endif
                }
                else {
                    reset_test_state();
                }
                break;

            case TEST_FIRMWARE_ERASE:
				g_source_remove(mTimer);
                if (result == RESULT_PASS) {
                    // all test passed!
                    printf("[%s:%s] %d,%d, TEST_FIRMWARE_ERASE test pass !\n", __FILE__, __func__);
                }
                reset_test_state();
                break;

            case TEST_RESET:
                // usb disconnected, reset test.
                reset_test_state();
                break;
        }
		threads_leave();
    }
}

static gint g_idle_thread_worker(void *args)
{
    struct USB_RESP_T *m = &mUsbRespT;
	
    printf("[%s:%s] resp=%d, result=%d \n", __FILE__, __func__, m->resp, m->result);
	g_UI_thread_worker(m->resp, m->result);
    return FALSE;
}

static void update_btn_test_show_info(int id, int result)
{
    char *disp = RESULT_FAILED_STR;
    GdkColor *color = &red;

	printf("[%s:%s],id=%d, ret=%d \n", __FILE__, __func__, id, result);

    switch (result) {
        case RESULT_PASS:
            disp = RESULT_PASS_STR;
            color = &green;
            break;

        case RESULT_FAILED:
            disp = RESULT_FAILED_STR;
            color = &red;
            break;

        case RESULT_RESET:
            disp = RESULT_RESET_STR;
            color = &black;
            break;

        case RESULT_TEST_KEY_OK:
            disp = RESULT_TEST_KEY_OK_STR;
            color = &black;
            break;

        case RESULT_TEST_KEY_CANCEL:
            disp = RESULT_TEST_KEY_CANCEL_STR;
            color = &black;
            break;

        case RESULT_TEST_OLED_LIGHT:
            disp = RESULT_TEST_OLED_LIGHT_STR;
            color = &black;
            break;

        case RESULT_TEST_OLED_DARK:
            disp = RESULT_TEST_OLED_DARK_STR;
            color = &black;
            break;

        case RESULT_TEST_FW_EARSE_START:
            disp = RESULT_TEST_FW_EARSE_START_STR;
            color = &black;
            break;
			
		case RESULT_TEST_TIME_OUT:
			disp = RESULT_TEST_TIMEOUT_STR;
			color = &red;
			break;

    }

    switch(id) {
        case TEST_HW_BTN_LEFT:
            gtk_widget_modify_bg(label_btn_left_result,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_btn_left_result), disp);
            break;

        case TEST_HW_BTN_RIGHT:
            gtk_widget_modify_bg(label_btn_right_result,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_btn_right_result),disp);
            break;

        case TEST_OLED_LIGHT:
            gtk_widget_modify_bg(label_oled_full_light,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_oled_full_light),disp);
            break;

        case TEST_OLED_DARK:
            gtk_widget_modify_bg(label_oled_full_dark,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_oled_full_dark),disp);
            break;

        case TEST_FIRMWARE_ERASE:
            gtk_widget_modify_bg(label_firmware_erase,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_firmware_erase),disp);
            break;
			
		case TEST_TIME_OUT:
		case TEST_RESET:
			//gtk_widget_modify_bg(label_firmware_erase,GTK_STATE_NORMAL, color);
			//gtk_label_set_markup(GTK_LABEL(label_firmware_erase),disp);
			break;
    }
}

static void usb_msg_callback(int resp_id, int result)
{
	mUsbRespT.resp = resp_id;
	mUsbRespT.result = result;
	g_idle_add_full(G_PRIORITY_HIGH_IDLE, g_idle_thread_worker, &mUsbRespT, NULL);
}

int main(int argc,char *argv[])
{
    threads_init();
    gtk_init(&argc,&argv);

    GtkBuilder *builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder,"builder.ui", NULL)) {
        printf("connot load builder.ui file!\n");
        return -1;
    }

    gdk_color_parse("grey51", &grey51);
    PangoFontDescription *button_font = pango_font_description_from_string("Sans");
    pango_font_description_set_size(button_font, 20*PANGO_SCALE);
    PangoFontDescription *test_item_font = pango_font_description_from_string("Sans");
    pango_font_description_set_size(test_item_font, 15*PANGO_SCALE);

    window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));
    button_start = GTK_BUTTON(gtk_builder_get_object(builder, "button_start"));
    gtk_button_set_label(GTK_BUTTON(button_start), START_BUTTON_GO_STR);
    gtk_widget_modify_font(GTK_BUTTON(button_start), button_font);


    g_signal_connect (button_start, "clicked", callback_for_hw_btn_test, (gpointer) window);

    // TEST_BTN LEFT 
    label_btn_left_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item1"));
    gtk_widget_modify_font(GTK_LABEL(label_btn_left_item), test_item_font);

    label_btn_left_result = GTK_LABEL(gtk_builder_get_object(builder, "label_result1"));
    gtk_widget_modify_bg(label_btn_left_result,GTK_STATE_NORMAL, &black);
    gtk_label_set_markup(GTK_LABEL(label_btn_left_result),RESULT_RESET_STR);
    gtk_widget_modify_font(GTK_LABEL(label_btn_left_result), test_item_font);

    // TEST_BTN RIGHT
    label_btn_right_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item2"));
    gtk_widget_modify_font(GTK_LABEL(label_btn_right_item), test_item_font);

    label_btn_right_result = GTK_LABEL(gtk_builder_get_object(builder, "label_result2"));
    gtk_label_set_markup(GTK_LABEL(label_btn_right_result),RESULT_RESET_STR);
    gtk_widget_modify_bg(label_btn_right_result,GTK_STATE_NORMAL, &black);
    gtk_widget_modify_font(GTK_LABEL(label_btn_right_result), test_item_font);


    // TEST_OLED_LIGHT
    label_oled_full_lg_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item3"));
    gtk_widget_modify_font(GTK_LABEL(label_oled_full_lg_item), test_item_font);

#ifndef TEST_BTN_AND_OLED_ONLY
    gtk_widget_modify_bg(label_oled_full_lg_item,GTK_STATE_NORMAL, &grey51);
    gtk_widget_set_sensitive(GTK_LABEL(label_oled_full_lg_item), FALSE);
#endif

    label_oled_full_light = GTK_LABEL(gtk_builder_get_object(builder, "label_result3"));
    gtk_label_set_markup(GTK_LABEL(label_oled_full_light),RESULT_RESET_STR);
    gtk_widget_modify_font(GTK_LABEL(label_oled_full_light), test_item_font);

#ifdef TEST_BTN_AND_OLED_ONLY
    gtk_widget_modify_bg(label_oled_full_light,GTK_STATE_NORMAL, &black);
#else
    gtk_widget_modify_bg(label_oled_full_light,GTK_STATE_NORMAL, &grey51);
    gtk_widget_set_sensitive(GTK_LABEL(label_oled_full_light), FALSE);
#endif

    // TEST_OLED_DARK
    label_oled_full_dk_item = GTK_GRID(gtk_builder_get_object(builder, "label_test_item4"));
    gtk_widget_modify_font(GTK_LABEL(label_oled_full_dk_item), test_item_font);

#ifndef TEST_BTN_AND_OLED_ONLY
    gtk_widget_modify_bg(label_oled_full_dk_item,GTK_STATE_NORMAL, &grey51);
    gtk_widget_set_sensitive(GTK_LABEL(label_oled_full_dk_item), FALSE);
#endif

    label_oled_full_dark= GTK_LABEL(gtk_builder_get_object(builder, "label_result4"));
    gtk_label_set_markup(GTK_LABEL(label_oled_full_dark),RESULT_RESET_STR);
    gtk_widget_modify_font(GTK_LABEL(label_oled_full_dark), test_item_font);

#ifdef TEST_BTN_AND_OLED_ONLY
    gtk_widget_modify_bg(label_oled_full_dark,GTK_STATE_NORMAL, &black);
#else
    gtk_widget_modify_bg(label_oled_full_dark,GTK_STATE_NORMAL, &grey51);
    gtk_widget_set_sensitive(GTK_LABEL(label_oled_full_dark), FALSE);
#endif

    // TEST_FIRMWARE_ERASE
    label_firmware_erase_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item5"));
    gtk_widget_modify_font(GTK_LABEL(label_firmware_erase_item), test_item_font);

#ifndef TEST_FULL_TEST
    gtk_widget_modify_bg(label_firmware_erase_item,GTK_STATE_NORMAL, &grey51);
    gtk_widget_set_sensitive(GTK_LABEL(label_firmware_erase_item), FALSE);
#endif

    label_firmware_erase = GTK_LABEL(gtk_builder_get_object(builder, "label_result5"));
    gtk_label_set_markup(GTK_LABEL(label_firmware_erase),RESULT_RESET_STR);
    gtk_widget_modify_font(GTK_LABEL(label_firmware_erase), test_item_font);

#ifdef TEST_FULL_TEST
    gtk_widget_modify_bg(label_firmware_erase,GTK_STATE_NORMAL, &black);
#else
    gtk_widget_modify_bg(label_firmware_erase,GTK_STATE_NORMAL, &grey51);
    gtk_widget_set_sensitive(GTK_LABEL(label_firmware_erase), FALSE);
#endif

    gtk_widget_show_all(window);

    threads_enter();
    gtk_main();
    return 0;
}

