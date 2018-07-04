#include <gtk/gtk.h>

#define FACTORY_DOWNLOAD
//#define FACTORY_TEST_STAGE1
//#define FACTORY_TEST_STAGE2
//#define FACTORY_TEST_FINAL

int init_main_ui(int argc, char *argv[]);

#ifdef FACTORY_DOWNLOAD
GtkButton *m_button_load_files;
GtkButton *m_button_start_download;
GtkTextView *m_textview_loaded_files;
GtkTextView *m_textview_download_logs;
GtkProgressBar *m_progressbar_download;
#endif

