#include <gtk/gtk.h>
#include "libusb.h"

int list_usb_devs(void);

static void activate (GtkApplication* app,
          gpointer        user_data)
{
  GtkWidget *window;//窗口类型指针
 
  window = gtk_application_window_new (app);//界面入口函数
  gtk_window_set_title (GTK_WINDOW (window), "Window");//window窗口标题设置
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);//window大小设置
  gtk_widget_show_all (window);//界面显示show（类Qt::widget->show()） interfaces_loop
}
 
int main (int argc, char **argv)
{
  GtkApplication *app;    //App应用入口
int status;
int ret;

ret = list_usb_devs();
printf("%s:list_usb_devs:%d\n",ret);

app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
//G_APPLICATION_FLAGS_NONE这个暂时还不知道是个什么样的宏  
g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);//信号回调函数
status = g_application_run (G_APPLICATION (app), argc, argv);//终端调用参数传入
g_object_unref (app);


return status;
}











