#include <stdio.h>
#include <gtk/gtk.h>

static void helloWorld(GtkWidget *wid, GtkWidget *win)
{
    GtkWidget *dialog = NULL;

    dialog = gtk_message_dialog_new(GTK_WINDOW (win), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "Hello World!");
    gtk_window_set_position(GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
    gtk_dialog_run(GTK_DIALOG (dialog));
    gtk_widget_destroy(dialog);
}

int main(int argc,char *argv[])
{
    static GdkColor red = {0, 0xffff, 0, 0};
    static GdkColor green = {0, 0, 0xffff, 0};
    static GdkColor blue = {0, 0, 0, 0xffff};
    static GdkColor yellow = {0, 0xffff, 0xffff, 0};
    static GdkColor cyan = {0, 0, 0xffff, 0xffff};

    static GdkColor grey51;//grid
    gdk_color_parse("grey51", &grey51);

    static GdkColor grey81;//item
    gdk_color_parse("grey51", &grey81);

    static GdkColor LightSkyBlue2;
    gdk_color_parse("LightSkyBlue2", &LightSkyBlue2);

    gtk_init(&argc,&argv);

    GtkBuilder *builder = gtk_builder_new();

    if (!gtk_builder_add_from_file(builder,"builder.ui", NULL)) {
        printf("connot load glade file!");
		return -1;
    }
	printf("load glade file success");

    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));

    GtkButton *button_start = GTK_BUTTON(gtk_builder_get_object(builder, "button_start"));

    GtkLabel *label_factory_test = GTK_LABEL(gtk_builder_get_object(builder, "label_factory_test"));
    //gtk_label_set_text(label, "factory test");
    gtk_widget_modify_bg(label_factory_test,GTK_STATE_NORMAL, &green);

    GtkLabel *label_title1 = GTK_LABEL(gtk_builder_get_object(builder, "label_title1"));
    gtk_widget_modify_bg(label_title1,GTK_STATE_NORMAL, &LightSkyBlue2);

    GtkLabel *label_title2 = GTK_LABEL(gtk_builder_get_object(builder, "label_title2"));
    gtk_widget_modify_bg(label_title2,GTK_STATE_NORMAL, &LightSkyBlue2);

    GtkGrid *gride_test_items = GTK_GRID(gtk_builder_get_object(builder, "grid_test_items"));
    gtk_widget_modify_bg(gride_test_items,GTK_STATE_NORMAL, &grey51);

    GtkLabel *label_test_item1 = GTK_LABEL(gtk_builder_get_object(builder, "label_test_item1"));
    gtk_widget_modify_bg(label_test_item1,GTK_STATE_NORMAL, &grey81);

    GtkLabel *label_result1 = GTK_LABEL(gtk_builder_get_object(builder, "label_result1"));
    // gtk_widget_modify_font(label_result1,GTK_STATE_NORMAL, &grey81);
    gtk_label_set_markup(GTK_LABEL(label_result1),"<span foreground='red'>result1</span>");

    GtkLabel *label_result2 = GTK_LABEL(gtk_builder_get_object(builder, "label_result2"));
    gtk_label_set_markup(GTK_LABEL(label_result2),"<span foreground='red'>result2</span>");

    GtkLabel *label_result3 = GTK_LABEL(gtk_builder_get_object(builder, "label_result3"));
    gtk_label_set_markup(GTK_LABEL(label_result3),"<span foreground='red'>result3</span>");

    GtkLabel *label_result4 = GTK_LABEL(gtk_builder_get_object(builder, "label_result4"));
    gtk_label_set_markup(GTK_LABEL(label_result4),"<span foreground='red'>result4</span>");

    GtkLabel *label_result5 = GTK_LABEL(gtk_builder_get_object(builder, "label_result5"));
    gtk_label_set_markup(GTK_LABEL(label_result5),"<span foreground='red'>result5</span>");

    g_signal_connect (button_start, "clicked", helloWorld, (gpointer) window);
    //g_signal_connect (button, "clicked", gtk_main_quit, NULL);

    const char *text = gtk_button_get_label(button_start);
    printf("text=%s\n", text);

    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
