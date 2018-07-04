#include <stdio.h>
#include <string.h>

#include "download_callback.h"
#include "factory_ui.h"

gchar mBinCfgFilePath[255];
gchar mBinCfgDirPath[255];

void parse_map_file(gchar *filepath, gchar* output_info)
{
    gchar temp[64];
    FILE *fpReader;
    GtkTextBuffer *buffer;

#ifdef __unix
    #ifdef __linux
        printf("Current System:Linux\n");
    #endif
#else
    #ifdef WINVER
        printf("Current System:Windows\n");
    #endif
#endif

    gint len_file_path = strlen(filepath);
    gint len_map_txt = strlen("map.txt");
    gint n_size = len_file_path - len_map_txt;

    //strncpy(dir_path,filepath, n_size);
    g_utf8_strncpy(mBinCfgDirPath,filepath, n_size);

    printf("###dir_path=%s\n", mBinCfgDirPath);

   // g_utf8_validate();

    fpReader=fopen(filepath,"r");

    if(fpReader==NULL)
    {
        printf("open %s failed\n", filepath);
        return NULL;
    }

    while(!feof(fpReader))
    {
        //fscanf(fpReader, "%s", temp);
        memset(temp, 0, sizeof(temp));
        fgets(temp, sizeof(temp)-1, fpReader);
        if (!strstr(temp,"FILE"))
        {
            if(strlen(temp) <= 0)
                continue;
            if(strlen(output_info) == 0)
            {
                strcpy(output_info,mBinCfgDirPath);
                strcat(output_info,temp);
            }
            else
            {
                strcat(output_info,mBinCfgDirPath);
                strcat(output_info,temp);
            }
        }
        //printf("output_info=%s",output_info);
    }
    fclose(fpReader);
}


void print_selected_filename(GtkWidget *file_chooser_dialog)
{
    GtkWidget *dialog;
    gchar *filename;

    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser_dialog));
    gtk_widget_destroy(file_chooser_dialog);
    printf("filename = %s", filename);
    dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,filename);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void show_selected_files_to_textview_panel(GtkWidget *file_chooser_dialog)
{
    GtkTextBuffer *buffer;
    gchar *filename;
    gchar infos_to_show[1024];

    memset(infos_to_show, 0, sizeof(infos_to_show));

    filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser_dialog));
    memcpy(mBinCfgFilePath,filename,strlen(filename));
    printf("mBinCfgFilePath = %s\n", mBinCfgFilePath);
    gtk_widget_destroy(file_chooser_dialog);

    parse_map_file(mBinCfgFilePath, infos_to_show);

    printf("infos_to_show = %s", infos_to_show);

    buffer = gtk_text_view_get_buffer(m_textview_loaded_files);
    gtk_text_buffer_set_text (buffer, infos_to_show, -1);

}


void load_files_on_clicked(GtkWidget *wid, GtkWidget *win)
{
    printf("load_files_on_clicked !!!\n");
    GtkWidget *dialog_file_chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;
    char *filename;

    dialog_file_chooser = gtk_file_chooser_dialog_new("Select File",
                          win,
                          GTK_FILE_CHOOSER_ACTION_OPEN,
                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                          NULL);

    printf("load_files_on_clicked show dialog_file_chooser !!!\n");
    res = gtk_dialog_run (GTK_DIALOG (dialog_file_chooser));
    printf("load_files_on_clicked res = %d\n", res);

    if (res == GTK_RESPONSE_ACCEPT)
    {
        //print_selected_filename(dialog_file_chooser);
        show_selected_files_to_textview_panel(dialog_file_chooser);
    }
    else
    {
        gtk_widget_destroy(dialog_file_chooser);
    }

}

void show_download_infos_to_textview_panel(char *infos) {
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(m_textview_download_logs);
    gtk_text_buffer_set_text (buffer, infos, -1);
}

int exec_shell_sh(char *action) {

	char script_file_path[256];
	char path[256];
	char log_buff[1024];
	FILE *fp;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(m_textview_download_logs);

    strcpy(script_file_path,mBinCfgDirPath);
#ifdef __unix
    #ifdef __linux
        printf("Current System:Linux\n");
        strcat(script_file_path, "download.sh");
    #endif
#else
    #ifdef WINVER
        printf("Current System:Windows\n");
        strcat(script_file_path, "download.bat");
    #endif
#endif


    //fp=popen(“cat /etc/passwd”,”r”);

    fp=popen("/home/android/projects/gtk_project/factory_test/test_download_files/download.sh /home/android/projects/gtk_project/factory_test/test_download_files/ b", "r");

    fgets(log_buff,sizeof(log_buff),fp);
    gtk_text_buffer_set_text (buffer, log_buff, -1);

    while(fgets(log_buff, sizeof(log_buff), fp) != NULL) {
        if (log_buff[strlen(log_buff) - 1] == '\n') {
            log_buff[strlen(log_buff) - 1] = '\0';
        }
        //printf("log_buff:%s", log_buff);
        gtk_text_buffer_set_text (buffer, log_buff, -1);
    }
    //while(!feof(fp)) {
   //     fgets(log_buff,sizeof(log_buff),fp);
        //printf("log_buff:%s", log_buff);
       // gtk_text_buffer_set_text (buffer, log_buff, -1);
   // }

    //show_download_infos_to_textview_panel(log_buff);

    pclose(fp);
	return 0;
}


void exec_stlink_download(char *action) {
	gint result = exec_shell_sh(action);
	printf("exec_shell_sh result = %d",result);
}

void start_download_on_clicked(GtkWidget *wid, GtkWidget *win)
{
    exec_stlink_download("b");
    exec_stlink_download("ftm-m");
    exec_stlink_download("ftm-f");
}
