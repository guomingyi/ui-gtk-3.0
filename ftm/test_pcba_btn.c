#include <stdio.h>
#include <gtk/gtk.h>

#include "usb.h"

#ifdef PCBA_TEST_3
#define PCBA_TEST_1
#define PCBA_TEST_2
#endif

#ifdef PCBA_TEST_2
#define PCBA_TEST_1
#endif

#ifndef PCBA_TEST_1
#define PCBA_TEST_1
#endif

#define BTN_TEST_TIMEOUT 5 // s

#define RESULT_PASS "<span foreground='black'>PASS</span>"
#define RESULT_FAILED "<span foreground='black'>FAIL</span>"
#define RESULT_EMPTY "<span foreground='black'> </span>"

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

static GdkColor red = {0, 0xffff, 0, 0};
static GdkColor green = {0, 0, 0xffff, 0};
static GdkColor blue = {0, 0, 0, 0xffff};
static GdkColor yellow = {0, 0xffff, 0xffff, 0};
static GdkColor cyan = {0, 0, 0xffff, 0xffff};

static GdkColor grey51;//grid
static GdkColor grey81;//item
static GdkColor LightSkyBlue2;

static guint mTimer = 0;
static int is_in_test = 0;
static int time_out = 0;

static int hw_btn_left_test_ok = 0;
static int hw_btn_right_test_ok = 0;

static void update_btn_test_show_info(int id, int result);


static void reset_test_state(void)
{
    hw_btn_left_test_ok = 0;
    hw_btn_right_test_ok = 0;
    is_in_test = 0;
    mTimer = 0;
    time_out = 0;
}

static void gtk_message_dialog_close_cb(GtkWidget *widget)
{
    printf("[%s:%s]\n", __FILE__, __func__);
    reset_test_state();
    gtk_widget_destroy(widget);
}


static gboolean hw_btn_test_time_out(gpointer* data)
{
    printf("[%s:%s] %d\n", __FILE__, __func__, time_out);

    if (mTimer == 0) 
        return FALSE;

    if (time_out++ == BTN_TEST_TIMEOUT -1) {
        if (!hw_btn_left_test_ok)
            update_btn_test_show_info(TEST_HW_BTN_LEFT, TEST_FAIL);

        if (!hw_btn_right_test_ok)
            update_btn_test_show_info(TEST_HW_BTN_RIGHT, TEST_FAIL);

        printf("[%s:%s] 5s exit\n", __FILE__, __func__);
        gtk_message_dialog_close_cb((GtkWidget *)data);
        return FALSE;
    }

    char dis_buf[64] = {0};
    sprintf(dis_buf, "Please press hw button: %ds", BTN_TEST_TIMEOUT - time_out);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG((GtkWidget *)data), dis_buf);
    return TRUE;
}

static void callback_for_hw_btn_test(GtkWidget *wid, GtkWidget *win)
{
    if (!is_in_test) {
        is_in_test = 1; // set start flag

        update_btn_test_show_info(TEST_HW_BTN_LEFT, TEST_RESET);
        update_btn_test_show_info(TEST_HW_BTN_RIGHT, TEST_RESET);

        char dis_buf[64] = {0};
        sprintf(dis_buf, "Please press hw button: %ds", BTN_TEST_TIMEOUT - time_out);
        GtkWidget *dialog = gtk_message_dialog_new_with_markup(
                GTK_WINDOW(win), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, dis_buf);
        gtk_window_set_position(GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
        g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_message_dialog_close_cb), dialog);

        mTimer = g_timeout_add(1000, (GSourceFunc)hw_btn_test_time_out, (void *)dialog);
        gtk_dialog_run(GTK_DIALOG(dialog));
    }
}

static void usb_msg_callback(int id, int result)
{
    printf("[%s:%s] %d,%d\n", __FILE__, __func__, id, result);

    if (is_in_test) {
        switch(id) {
            case TEST_HW_BTN_LEFT:
                update_btn_test_show_info(TEST_HW_BTN_LEFT, TEST_PASS);
                hw_btn_left_test_ok = 1;
                break;
            case TEST_HW_BTN_RIGHT:
                update_btn_test_show_info(TEST_HW_BTN_RIGHT, TEST_PASS);
                hw_btn_right_test_ok = 1;
                break;
        }
    }
}

static void update_btn_test_show_info(int id, int result)
{
    char *disp = (result == 0 ? RESULT_PASS : (result == -1 ? RESULT_FAILED : RESULT_EMPTY));
    GdkColor *color = (result == 0 ? &green : (result == -1 ? &red : &grey51));

    switch(id) {
        case TEST_HW_BTN_LEFT:
            gtk_widget_modify_bg(label_btn_left_result,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_btn_left_result), disp);
            break;
        case TEST_HW_BTN_RIGHT:
            gtk_widget_modify_bg(label_btn_right_result,GTK_STATE_NORMAL, color);
            gtk_label_set_markup(GTK_LABEL(label_btn_right_result),disp);
            break;
    }
}

int main(int argc,char *argv[])
{
    gtk_init(&argc,&argv);
    GtkBuilder *builder = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder,"builder.ui", NULL)) {
        printf("connot load builder.ui file!");
        return -1;
    }

    gdk_color_parse("grey51", &grey51);

    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));
    GtkButton *button_start = GTK_BUTTON(gtk_builder_get_object(builder, "button_start"));
    g_signal_connect (button_start, "clicked", callback_for_hw_btn_test, (gpointer) window);

    // TEST_BTN LEFT 
    label_btn_left_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item1"));
    label_btn_left_result = GTK_LABEL(gtk_builder_get_object(builder, "label_result1"));
    gtk_widget_modify_bg(label_btn_left_result,GTK_STATE_NORMAL, &grey51);
    gtk_label_set_markup(GTK_LABEL(label_btn_left_result),RESULT_EMPTY);

    // TEST_BTN RIGHT
    label_btn_right_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item2"));
    label_btn_right_result = GTK_LABEL(gtk_builder_get_object(builder, "label_result2"));
    gtk_label_set_markup(GTK_LABEL(label_btn_right_result),RESULT_EMPTY);
    gtk_widget_modify_bg(label_btn_right_result,GTK_STATE_NORMAL, &grey51);

    // TEST_OLED_LIGHT
    label_oled_full_lg_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item3"));
    gtk_widget_modify_bg(label_oled_full_lg_item,GTK_STATE_NORMAL, &grey51);
    gtk_label_set_markup(GTK_LABEL(label_oled_full_lg_item),RESULT_EMPTY);

    label_oled_full_light = GTK_LABEL(gtk_builder_get_object(builder, "label_result3"));
    gtk_label_set_markup(GTK_LABEL(label_oled_full_light),RESULT_EMPTY);
    gtk_widget_modify_bg(label_oled_full_light,GTK_STATE_NORMAL, &grey51);

    // TEST_OLED_DARK
    label_oled_full_dk_item = GTK_GRID(gtk_builder_get_object(builder, "label_test_item4"));
    gtk_widget_modify_bg(label_oled_full_dk_item,GTK_STATE_NORMAL, &grey51);
    gtk_label_set_markup(GTK_LABEL(label_oled_full_dk_item),RESULT_EMPTY);

    label_oled_full_dark= GTK_LABEL(gtk_builder_get_object(builder, "label_result4"));
    gtk_label_set_markup(GTK_LABEL(label_oled_full_dark),RESULT_EMPTY);
    gtk_widget_modify_bg(label_oled_full_dark,GTK_STATE_NORMAL, &grey51);

    // TEST_FIRMWARE_ERASE
    label_firmware_erase_item = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item5"));
    gtk_widget_modify_bg(label_firmware_erase_item,GTK_STATE_NORMAL, &grey51);
    gtk_label_set_markup(GTK_LABEL(label_firmware_erase_item),RESULT_EMPTY);

    label_firmware_erase = GTK_LABEL(gtk_builder_get_object(builder, "label_result5"));
    gtk_label_set_markup(GTK_LABEL(label_firmware_erase),RESULT_EMPTY);
    gtk_widget_modify_bg(label_firmware_erase,GTK_STATE_NORMAL, &grey51);

    // for usb loop.
    if (register_usb_callback(usb_msg_callback) == NULL) {
        printf("[%s] register_usb_callback failed!\n",__func__);
    }

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
