#include <stdio.h>
#include <gtk/gtk.h>

#include "usb.h"

static int usb_init_success = 0;

void setUsbInit(int s)
{
	usb_init_success = s;
}
int usb_init(void)
{   
    int ret = -1;
	
    printf("%s()\n", __func__);
    ret = usbdev_init();
	return ret;
}

int usb_enumerate(int vid, int pid)
{
    int ret = -1;
	
    printf("%s()\n", __func__);
    ret = usbdev_enumerate(vid, pid);
	return ret;
}

int usb_write(char *data)
{
    int ret = -1;
	
	if (usb_init_success) {
	    ret = usb_msg_out(data);
	}
	return ret;
}

int usb_read(char *data)
{
    int ret = -1;
	
	if (usb_init_success) {
	    ret = usb_msg_in(data);
	}
	return ret;
}

int usb_close(void)
{
    int ret = -1;

	if (usb_init_success) {
		printf("%s()\n", __func__);
		ret = usbdev_close();
		usb_init_success = 0;
	}
	return ret;
}

static int usb_msg_update_ui(void *data, char *msg) 
{
    FuncPtrII cb = (FuncPtrII)data;
	
    if (strncmp(msg, "btn#Y", 5) == 0) {
		cb(TEST_HW_BTN_LEFT, RESULT_PASS);
		goto out;
    }

    if (strncmp(msg, "btn#N", 5) == 0) {
		cb(TEST_HW_BTN_RIGHT, RESULT_PASS);
		goto out;
    }

    if (strncmp(msg, "light_screen_good", 17) == 0) {
		cb(TEST_OLED_LIGHT, RESULT_PASS);
		goto out;
    }
	
    if (strncmp(msg, "light_screen_bad", 16) == 0) {
		cb(TEST_OLED_LIGHT, RESULT_FAILED);
		goto out;
    }

    if (strncmp(msg, "dark_screen_good", 16) == 0) {
		cb(TEST_OLED_DARK, RESULT_PASS);
		goto out;
    }
	
    if (strncmp(msg, "dark_screen_bad", 15) == 0) {
		cb(TEST_OLED_DARK, RESULT_FAILED);
		goto out;
    }

	if (strcmp(msg, "#firmware erase success#") == 0) {
		usb_close();
        cb(TEST_FIRMWARE_ERASE, RESULT_PASS);
		return 1;
	}

	if (strcmp(msg, "1Erase fw") == 0) {
		usb_close();
        cb(TEST_FIRMWARE_ERASE, RESULT_PASS);
		return 1;
	}

	if (strcmp(msg, "usb-disconnect") == 0) {
		usb_close();
		return 1;
	}

out:
	return 0;
}

static GThreadFunc usb_update_monitor(gpointer data) 
{
    char buf[64] = {0};
	FuncPtrII cb = (FuncPtrII)data;
	
	printf("%s: start\n", __func__);
	
    do {
		printf("[%s:%s] usb_read wait.. \n", __FILE__, __func__);
        if (usb_read(buf) < 0) {
            printf("err! usb read failed, goto close usb!\n");
			usb_close();
			cb(TEST_RESET, RESULT_RESET);
            break;
        }
		
		printf("%s: receiver: %s\n", __func__,buf);
        if (usb_msg_update_ui(cb, buf)) {
            break;
		}
    }
    while (1);

    printf("%s: g_thread_exit()\n", __func__);
	g_thread_exit(NULL);
    return NULL;
}


void *register_usb_callback(void *cb) 
{
	if (usb_init() == 0 && usb_enumerate(VID, PID) == 0) {
	    usb_init_success = 1;        
	}
	else {
		printf("%s: failed!\n", __func__);
	    return NULL;
	}
	return (void *)g_thread_new("usb-communicate-test", usb_update_monitor, cb);
}

















