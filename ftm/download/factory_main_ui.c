#include <stdio.h>
#include <gtk/gtk.h>
#include "factory_ui.h"
#include "download_callback.h"


int init_main_ui(int argc, char *argv[])
{    
    gtk_init(&argc,&argv);
	
    GtkBuilder *builder = gtk_builder_new();
#ifdef FACTORY_DOWNLOAD
    if (!gtk_builder_add_from_file(builder,"factory_download.glade", NULL))
    {
        printf("Error:connot load factory_download.glade file!");
        return -1;
    } else {
        printf("GtkBuilder load factory_download.glade file success!!!");
    }
#elif defined FACTORY_TEST_STEP1
    if (!gtk_builder_add_from_file(builder,"factory_test_step1.glade", NULL))
    {
        printf("Error:connot load factory_test_step1.glade file!");
        return -1;
    } else {
        printf("GtkBuilder load factory_test_step1.glade file success!");
    }
#elif defined FACTORY_TEST_STEP2
    if (!gtk_builder_add_from_file(builder,"factory_test_step2.glade", NULL))
    {
        printf("Error:connot load factory_test_step2.glade file!");
        return -1;
    } else {
        printf("GtkBuilder load factory_test_step2.glade file success!!!");
    }
#elifdef FACTORY_TEST_FINAL
    if (!gtk_builder_add_from_file(builder,"factory_test_final.glade", NULL))
    {
        printf("Error:connot load factory_test_final.glade file!");
        return -1;
    } else {
        printf("GtkBuilder load factory_test_final.glade file success!!!");
    }
#endif // FACTORY_DOWNLOAD
    GtkWidget *m_window_main = GTK_WIDGET(gtk_builder_get_object(builder,"window_main"));
    GtkFixed  *m_fixed_main = GTK_FIXED(gtk_builder_get_object(builder,"fixed"));

#ifdef FACTORY_DOWNLOAD
    init_download_ui(builder, m_window_main);
#endif // FACTORY_DOWNLOAD

    gtk_widget_show_all(m_window_main);

    gtk_main();
}
