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
    //gchar filename[255] = {0};
    gchar *filename;
    gchar infos_to_show[1024];

    memset(infos_to_show, 0, sizeof(infos_to_show));

    char *curr = g_get_current_dir();
		
	printf("g_get_current_dir = %s\n", curr);

    //filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser_dialog));
    //gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(file_chooser_dialog), filename);
	
    //memcpy(mBinCfgFilePath,filename,strlen(filename));
    sprintf(mBinCfgFilePath,"%s\\sw\\maps.txt", curr);
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


static gint g_idle_thread_worker(void *args)
{	
    printf("[%s:%s]\n", __FILE__, __func__);
	
    char *p = (char *)args;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(m_textview_download_logs);
	gtk_text_buffer_set_text(buffer, p, -1);
    return FALSE;
}

static char log_buff[1024];

int exec_shell_sh(char *action) {

	char path[256];
	char buf[256] = {0};
    char *p = buf, *q = log_buff;
	
	char cmd[256] = {0};
	FILE *fp;

    printf("[%s:%s] enter \n", __FILE__,__func__);

    char *curr = g_get_current_dir();
    sprintf(cmd, "%s\\download.bat %s\\sw %s", curr, curr, action);
    printf("[%s:%s]----cmd----:%s\n", __FILE__,__func__,cmd);
	
    fp = popen(cmd, "r");
    while(fgets(p, 256, fp) != NULL) {
        if (p[256 - 1] == '\n') {
            p[256 - 1] = '\0';
        }
		printf("%s\n", p);
    }
    pclose(fp);
    
    g_idle_add_full(G_PRIORITY_HIGH_IDLE, g_idle_thread_worker, log_buff, NULL);
	return 0;
}


void exec_stlink_download(char *action) {
	gint result = exec_shell_sh(action);
}

void *exec_download_thread_callback(void *args) {
printf("[%s:%d].\n", __FILE__,__LINE__);

	for (int i = 0; i < 20; i++) {
		exec_stlink_download("b");
		exec_stlink_download("ftm-m");
		exec_stlink_download("ftm-f");
	}

	g_thread_exit(NULL);
    return NULL;
}

void start_download_on_clicked(GtkWidget *wid, GtkWidget *win)
{
    printf("[%s:%s]ENTER \n", __FILE__,__func__);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(m_textview_download_logs);
    gtk_text_buffer_set_text (buffer, "", -1);
	g_thread_new("download thread",exec_download_thread_callback, NULL);
}
