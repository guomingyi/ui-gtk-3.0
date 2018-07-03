#include <stdio.h>
#include <gtk/gtk.h>

#include "usb.h"

static int usb_init_success = 0;

int usb_init(void)
{
    return usbdev_init();
}

int usb_enumerate(int vid, int pid)
{
    return usbdev_enumerate(vid, pid);
}

int usb_write(char *data)
{
	if (usb_init_success) {
	    return usb_msg_out(data);
	}
	return -1;
}

int usb_read(char *data)
{
   if (usb_init_success) {
	   return usb_msg_in(data);
   }
   return -1;

}

int usb_close(void)
{
    return usbdev_close();
}

static void usb_msg_update_ui(void *data, char *msg) 
{
    FuncPtrII cb = (FuncPtrII)data;
	
    if (msg != NULL && strncmp(msg, "btn#Y", 5) == 0) {
		cb(TEST_HW_BTN_LEFT, TEST_PASS);
    }
	else
    if (msg != NULL && strncmp(msg, "btn#N", 5) == 0) {
		cb(TEST_HW_BTN_RIGHT, TEST_PASS);
    }
}

static GThreadFunc usb_update_monitor(gpointer data) 
{
    char buf[64] = {0};
	
    for (;;) {
        if (usb_read(buf) < 0) {
            printf("err! usb read failed!\n");
            break;
        }
        usb_msg_update_ui(data, buf);
    }

	g_thread_exit(NULL);
    return NULL;
}


void *register_usb_callback(void *cb) 
{
    if (usb_init() == 0 && usb_enumerate(VID, PID) == 0) {
        usb_init_success = 1;        
    }
	else {
        return NULL;
	}
    return (void *)g_thread_new("usb-communicate-test", usb_update_monitor, cb);
}


















