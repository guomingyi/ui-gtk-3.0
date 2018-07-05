#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <libusb.h>

#include "usb.h"

static int action = 0;
struct libusb_device_handle *handle = NULL;

// host -> device
int usb_msg_out(char *buf)
{
    char data[64] = {0};
    int actual_len = 0;

    if (handle == NULL) { 
		printf("[%s] EEEEEEEEEEEEEEE out \r\n",__func__);
		return -1; 
	}
	
    sprintf(data, "%s", buf);
    libusb_bulk_transfer(handle, edp2out, data, 64, &actual_len, 0);
    if (actual_len > 0) {
		printf("[%s] send data: [%s] success.\r\n",__func__, buf);
        return 0;
    }
    printf("[%s] send data err\r\n",__func__);
    return -1;
}

// device -> host
int usb_msg_in(char *buf)
{
    char data[64] = {0};
    int actual_len = 0;

	if (handle == NULL) { 
		printf("[%s] EEEEEEEEEEEEEEE in \r\n",__func__);
		return -1; 
	}
	
	printf("[%s] wait read..\r\n",__func__);
    libusb_bulk_transfer(handle, edp2in, data, 64, &actual_len, 0);
    if (actual_len > 0) {
        memcpy(buf, data, 64);
        return 0;
    }
    buf[0] = '\0';
    printf("[%s] Receiver err\r\n",__func__);
    return -1;
}

static libusb_context *usb_ctx = NULL;

int usbdev_init(void)
{
    libusb_device **devs;
    ssize_t cnt;
    int ret;

    ret = libusb_init(&usb_ctx);
    if (ret < 0) {
        printf("[%s] err\r\n",__func__);
		return -1;
	}

    libusb_set_debug(usb_ctx, LIBUSB_LOG_LEVEL_INFO);
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        printf("[%s] no usb dev on bus\r\n",__func__);
        return  -1;
    }
	
	libusb_free_device_list(devs, 1);
	
	printf("[%s:%s] success!\r\n",__FILE__, __func__);
	return 0;
}

int usbdev_enumerate(int vid, int pid)
{
	int ret;
	
	if (usb_ctx == NULL) {
        printf("[%s] usb context not init\r\n",__func__);
		goto exit;
	}
		
    handle = libusb_open_device_with_vid_pid(usb_ctx, vid, pid);
    if (handle == NULL) {
        printf("[%s] cant't open device\r\n",__func__);
        goto exit;
    }

    if (libusb_kernel_driver_active(handle, 0) == 1) {
        if (libusb_detach_kernel_driver(handle, 0) != 0) {
            printf("[%s] detached kernel driver err\r\n",__func__);
            goto exit;
        }
    }

    ret = libusb_claim_interface(handle, 0);
    if (ret < 0) {
        printf("[%s] can't claim interface\r\n",__func__);
        goto exit;
    } 
	
	return 0;
	
exit:
    return -1;
}

int usbdev_close(void)
{
	if (handle) {
		libusb_close(handle);
		libusb_exit(NULL);
		handle = NULL;
	    usb_ctx = NULL;
	}

	return 0;
}

//int main(int argc, char* argv[])
int main2(int argc, char* argv[])
{
    libusb_device **devs, *dev;
    ssize_t cnt;
    struct libusb_device_descriptor desc; 
    libusb_context *ctx = NULL;
    char buf[64];
    int ret, i;
    int found = 0;

    for(i = 0; i < argc; i++) {
        printf("argv[%d]:%s\n", i, argv[i]);
        if (strcmp(argv[i], "erase") == 0) {
            action = 1;
            break;
        }
    }

    ret = libusb_init(&ctx);
    if (ret < 0)
        return -1;

    libusb_set_debug(ctx, 3);
    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        printf("no usb dev on bus\r\n");
        return  -1;
    }

    i = 0;
    while((dev = devs[i++]) != NULL) {
        ret = libusb_get_device_descriptor(dev,&desc);
        if (ret < 0) {
            printf("failed to get device descriptor");
            goto exit;
        }

        if ((desc.idVendor == VID) && (desc.idProduct == PID)) {
            printf("bLength: 0x%04x\r\n", desc.bLength);
            printf("bDescriptorType: 0x%04x\r\n", desc.bDescriptorType);
            printf("bcdUSB: 0x%04x\r\n", desc.bcdUSB);
            printf("bDeviceClass: 0x%04x\r\n", desc.bDeviceClass);
            printf("bDeviceSubClass: 0x%04x\r\n", desc.bDeviceSubClass);
            printf("bDeviceProtocol: 0x%04x\r\n", desc.bDeviceProtocol);
            printf("bMaxPacketSize0: 0x%04x\r\n", desc.bMaxPacketSize0);
            printf("vendor id: 0x%04x\r\n", desc.idVendor);
            printf("product id: 0x%04x\r\n", desc.idProduct);
            printf("bcdDevice: 0x%04x\r\n", desc.bcdDevice);
            printf("iManufacturer: 0x%04x\r\n", desc.iManufacturer);
            printf("iProduct: 0x%04x\r\n", desc.iProduct);
            printf("iSerialNumber: 0x%04x\r\n", desc.iSerialNumber);
            printf("bNumConfigurations: 0x%04x\r\n", desc.bNumConfigurations);
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("-------------- NOT FOUND DEVICE: (0x%04x:0x%04x) ---------------\n\n", VID, PID);
        goto exit;
    }

    handle = libusb_open_device_with_vid_pid(ctx, VID, PID);
    if (handle == NULL) {
        printf("cant't open device\r\n");
        goto exit;
    }

    libusb_free_device_list(devs, 1);

    if (libusb_kernel_driver_active(handle, 0) == 1) {
        if (libusb_detach_kernel_driver(handle, 0) != 0) {
            printf("detached kernel driver err!\r\n");
            goto exit;
        }
    }

    ret = libusb_claim_interface(handle, 0);
    if (ret < 0) {
        printf("can't claim interface\r\n");
        goto exit;
    } 
  
    if (1 == action) {
        usb_msg_out("Erase fw");
        usb_msg_in(buf);
        if (buf[0] == 0) {
            printf("*********exception exit.**********\r\n");
        }
        goto exit;
    }

    do {
        usb_msg_out("test btn.");
        usb_msg_in(buf);
       // usleep(1000*100);
	    sleep(1);
    }
    while(1);

exit:
    printf("free device list\r\n");
    if (handle)
        libusb_close(handle);
    libusb_exit(NULL);
    return 0;
}
