#include <stdio.h>
#include <string.h>

#include "download_callback.h"
#include "factory_ui.h"

/***********************************************************************************************************************/

static char mBinCfgDirPath[255];
static char bin_file_to_show[1024];
static char log_buff[64][256];

typedef struct PROCESS_BAR_T {
    float f;
    char t[32];
};

typedef struct TEXT_VIEW_T {
    char t[256];
    int size;
};

static struct TEXT_VIEW_T mText_view;
static struct PROCESS_BAR_T mProcess_barUpdate;

static guint mTimer = 0;
static int timer_out_count = 0;

#define float_covent(x,xx) (((100.0/x)*xx)/100.0)

#define LOG_TEXT_UPDATE(x,xx) \
    do { \
        g_idle_add_full(G_PRIORITY_HIGH_IDLE, g_idle_thread_worker_log, x, NULL); \
    } while(0)


#define PROCESS_BAR_UPDATE(x,xx) \
    do { \
        mProcess_barUpdate.f = x; \
        if (xx != NULL) \
        strcpy(mProcess_barUpdate.t, xx); \
        else \
        memset(mProcess_barUpdate.t, 0, sizeof(mProcess_barUpdate.t)); \
        g_idle_add_full(G_PRIORITY_HIGH_IDLE, g_idle_thread_worker_processbar, &mProcess_barUpdate, NULL); \
    } while(0)

/***********************************************************************************************************************/

static void parse_map_file(gchar *filepath, gchar* output_info)
{
    gchar temp[64];
    FILE *fpReader;
    char *bindir = mBinCfgDirPath;
    int n = 0;

#ifdef __linux
    printf("HOST System: Linux\n");
#else
    printf("HOST System: Windows\n");
#endif

    gint len_file_path = strlen(filepath);
    gint len_map_txt = strlen("map.txt");
    gint n_size = len_file_path - len_map_txt;

    strncpy(bindir, filepath, n_size - 1);
    fpReader=fopen(filepath,"r");
    if(fpReader==NULL) {
        printf("open %s failed\n", filepath);
        return NULL;
    }

    while(!feof(fpReader) && n++ < 3) {
        memset(temp, 0, sizeof(temp));
        fgets(temp, sizeof(temp)-1, fpReader);
        if (!strstr(temp,"FILE")) {
            if(strlen(temp) <= 0)
                continue;
            if(strlen(output_info) == 0) {
                strcpy(output_info,bindir);
                strcat(output_info,"\\");
                strcat(output_info,temp);
            }
            else {
                strcat(output_info,bindir);
                strcat(output_info,"\\");
                strcat(output_info,temp);
            }
        }
    }
    fclose(fpReader);
}

static void save_filename_to_tmp_file(char *filename) 
{
    FILE *fp;
    char *s= filename;
    char c = '\n';

    remove("load.txt");
    fp = fopen("load.txt","a");
    fprintf(fp,"%s",s);
    fprintf(fp,"%c",c);
    fclose(fp);
}

void load_filename_from_tmp_file(void) 
{
    FILE *fp;
    char s[128] = {0};

    fp = fopen("load.txt","r");
    if (fp == NULL) 
        return;

    fscanf(fp, "%s\n", s);
    fclose(fp);

    if (strlen(s) > 0) {
        parse_map_file(s, bin_file_to_show);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(m_textview_loaded_files), bin_file_to_show, -1);
    }
}

static void show_selected_files_to_textview_panel(GtkWidget *file_chooser_dialog)
{
    gchar *filename;
    gsize readt,writet;

    memset(bin_file_to_show, 0, sizeof(bin_file_to_show));
    filename = g_filename_display_name(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser_dialog)));
    if (filename != NULL) {
        parse_map_file(filename, bin_file_to_show);
        gtk_text_buffer_set_text(gtk_text_view_get_buffer(m_textview_loaded_files), bin_file_to_show, -1);
        save_filename_to_tmp_file(filename);
    }
}

static void show_download_infos_to_textview_panel(char *infos) {
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(m_textview_download_logs);
    gtk_text_buffer_set_text (buffer, infos, -1);
}

static void insert_text_to_textview(gchar * data)
{
    GtkTextBuffer *buffer;
    GtkTextIter end;
    gchar *escape, *text;
    escape = g_strescape (data, NULL);
    text = g_strconcat (escape, "\n", NULL);
    buffer = gtk_text_view_get_buffer(m_textview_download_logs);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_insert(buffer, &end, text, -1);
    g_free (escape);
    g_free (text);
}

static gint g_idle_thread_worker_settext_size(void *args)
{	
    int *s = (int *)args;
    if (*s > 0) {
        PangoFontDescription *f = pango_font_description_from_string("Sans");
        pango_font_description_set_size(f, *s*PANGO_SCALE);
        gtk_widget_modify_font(GTK_TEXT_VIEW(m_textview_download_logs), f);
    }
    return FALSE;
}

static void setLogTextViewSize(int s) 
{
	g_idle_add_full(G_PRIORITY_HIGH_IDLE, g_idle_thread_worker_settext_size, &s, NULL);
}

static gint g_idle_thread_worker_log(void *args)
{	
    insert_text_to_textview((char *)args);
    return FALSE;
}

static int exec_shell_sh(char *action) 
{
    int i = 0;
    char buf[256];
    char *p = buf;
    char cmd[256] = {0};
    char *bindir = mBinCfgDirPath;

    sprintf(cmd, "%s\\download.bat %s %s", g_get_current_dir(), bindir, action);

    FILE *fp = popen(cmd, "r");
    while(fgets(p, 256, fp) != NULL) {
        if (p[256 - 1] == '\n') {
            p[256 - 1] = '\0';
        }
        strncpy(log_buff[i++], p, strlen(p));
    }
    pclose(fp);

    for (int j = 0; j < i; j++) {
        LOG_TEXT_UPDATE(log_buff[j], 10);
        printf("%s: log_buff[%d]:%s\n", __func__, j, log_buff[j]);
    }

    for (int j = 0; j < 64; j++) {
        if (strstr(log_buff[j], "No ST-LINK detected") != NULL) {
            goto failed;
        }
        if (strstr(log_buff[j], "Programming Complete") != NULL) {
            return 0;
        }
    }

failed:
    printf("[%s] Programming failed! cmd:%s\n", __func__, cmd);
    return -1;
}

static gint g_idle_thread_worker_processbar(void *args)
{
    struct PROCESS_BAR_T *m = (struct PROCESS_BAR_T *)args;
    char buf[128] = {0};

    if (m->f > 0) {
        gtk_progress_bar_set_fraction(m_progressbar_download, m->f);
    }

    if (strlen(m->t) > 0) {
        gtk_progress_bar_set_text(m_progressbar_download, m->t);
    }
    else {
        sprintf(buf, "%d%%", (int)(m->f*100));
        gtk_progress_bar_set_text(m_progressbar_download, buf);
    }

    return FALSE;
}

static gint g_idle_thread_worker_ui_reset(void *args)
{
    gtk_widget_set_sensitive(GTK_BUTTON(m_button_load_files), TRUE);
    gtk_widget_set_sensitive(GTK_BUTTON(m_button_start_download), TRUE);
    return FALSE;
}

static gboolean downlaod_process_time_out(gpointer* data)
{
    float s = 0.0;

    if (mTimer > 0) {
        timer_out_count++;
        //printf("%s: %d\n",__func__, timer_out_count);
        s = float_covent(*((int *)(data)), timer_out_count);
        PROCESS_BAR_UPDATE(s < 1.0 ? s : 1.0, NULL);
        return TRUE;
    }
    return FALSE;
}


static void *exec_download_thread_callback(void *args) {
    char *cmd[3];
    char *bin[3];
    char *p = bin_file_to_show;
    int tim;

    if (strstr(p, "ftm.bin") != NULL) {
        cmd[0] = "b";
        cmd[1] = "ftm-m";
        cmd[2] = "ftm-f";

        bin[0] = "bootloader";
        bin[1] = "metadata-ftm";
        bin[2] = "firmware-ftm";
        tim = 140;
    } else {
        cmd[0] = "b";
        cmd[1] = "m";
        cmd[2] = "f";

        bin[0] = "bootloader";
        bin[1] = "metadata";
        bin[2] = "firmware";
        tim = 291;
    }

    timer_out_count = 0;
    mTimer = g_timeout_add(100, (GSourceFunc)downlaod_process_time_out, &tim);

    for (int i = 0; i < 3; i++) {
        if (exec_shell_sh(cmd[i]) < 0) {
            //setLogTextViewSize(20);
            PROCESS_BAR_UPDATE(-1, "FAILED");
            LOG_TEXT_UPDATE("***************** DOWNLAOD FAILED! *****************\n", 20);
            goto exit;
        }
    }

    PROCESS_BAR_UPDATE(1.0, "SUCCESS");
    LOG_TEXT_UPDATE("***************** DOWNLAOD SUCCESS *****************\n", 10);

exit:
    g_source_remove(mTimer);
    mTimer = 0;

    g_idle_add_full(G_PRIORITY_HIGH_IDLE, g_idle_thread_worker_ui_reset, NULL, NULL);
    g_thread_exit(NULL);
    return NULL;
}

void load_files_on_clicked(GtkWidget *wid, GtkWidget *win)
{
    GtkWidget *dialog_file_chooser;
    gint res = 0;

    dialog_file_chooser = gtk_file_chooser_dialog_new("Select File",
            win,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
            NULL);

    res = gtk_dialog_run (GTK_DIALOG (dialog_file_chooser));
    if (res == GTK_RESPONSE_ACCEPT) {
        show_selected_files_to_textview_panel(dialog_file_chooser);
    }
    else {
        printf("[%s] load_files failed: res:%d\n", __func__, res);
    }
    gtk_widget_destroy(dialog_file_chooser);
}

void start_download_on_clicked(GtkWidget *wid, GtkWidget *win)
{
    for (int j = 0; j < 64; j++) {
        memset(log_buff[j], 0, sizeof(log_buff[j]));
    }

    PROCESS_BAR_UPDATE(0, " ");
    gtk_widget_set_sensitive(GTK_BUTTON(m_button_load_files), FALSE);
    gtk_widget_set_sensitive(GTK_BUTTON(m_button_start_download), FALSE);

    setLogTextViewSize(10);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(m_textview_download_logs), "***************** START DOWNLAOD *****************\n", -1);
    g_thread_new("download thread",exec_download_thread_callback, NULL);
}
