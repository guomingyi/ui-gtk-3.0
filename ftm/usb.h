#ifndef __usb_h__
#define __usb_h__

#define TEST_RESET 1
#define TEST_PASS 0
#define TEST_FAIL -1

#define edp2in 0x81
#define edp2out 0x01

#define VID 0x534c
#define PID 0x0001

typedef void (*FuncPtrII)(int, int);
//GThreadFunc usb_thread_func(gpointer data) ;



enum {
	TEST_HW_BTN_LEFT,
	TEST_HW_BTN_RIGHT,
	TEST_OLED_LIGHT,
	TEST_OLED_DARK,
	TEST_FIRMWARE_ERASE
};

int usbdev_init(void);
int usbdev_enumerate(int vid, int pid);
int usb_msg_out(char *buf);
int usb_msg_in(char *buf);
int usbdev_close(void);

void *register_usb_callback(void *cb) ;


#endif
