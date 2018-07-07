#include <stdio.h>
#include <gtk/gtk.h>

#include "download_callback.h"
#include "factory_ui.h"

void init_download_callbacks(GtkWidget *win)
{
    g_signal_connect (m_button_load_files, "clicked", G_CALLBACK (load_files_on_clicked), (gpointer)win);
    g_signal_connect (m_button_start_download, "clicked", G_CALLBACK (start_download_on_clicked), (gpointer)win);
}

int init_download_ui(GtkBuilder *builder, GtkWidget *win)
{
    if (m_button_load_files == NULL)
    {
        m_button_load_files = GTK_BUTTON(gtk_builder_get_object(builder,"button_load_files"));
    }

    if (m_button_start_download == NULL)
    {
        m_button_start_download = GTK_BUTTON(gtk_builder_get_object(builder,"button_start_download"));
    }

    if (m_textview_loaded_files == NULL)
    {
        m_textview_loaded_files = GTK_TEXT_VIEW(gtk_builder_get_object(builder,"textview_loaded_files"));
    }

    if (m_textview_download_logs == NULL)
    {
        m_textview_download_logs = GTK_TEXT_VIEW(gtk_builder_get_object(builder,"textview_download_logs"));
    }

    if (m_progressbar_download == NULL)
    {
        m_progressbar_download = GTK_PROGRESS_BAR(gtk_builder_get_object(builder,"progressbar_download"));
    }

    load_filename_from_tmp_file();
    init_download_callbacks(win);
}
