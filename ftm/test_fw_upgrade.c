#include <stdio.h>
#include <gtk/gtk.h>
#include "usb.h"
#include<getopt.h>

#ifdef __linux
#include <semaphore.h>
#else
#include <process.h>
#include <windows.h>  
#endif

#define READ_BUFF 64

#define BIN_FILE_PATH "/home/gmy/work/justsafe/mcu-tag-v1.6.1/build-bak/firmware-v1.0-upgrade.bin"

static int step_flag = 0;

#ifdef __linux
static sem_t sem;
#else
static HANDLE sem;
#endif

typedef void (*FuncPtrIII)(char *);

enum {
    REQ_INITIAL = 0,
    REQ_ERASE,
    REQ_UPGRADE_FW,
    REQ_CHECK,
};

enum {
    RES_SUCCESS = 0,
    RES_ERROR = -1,
};

void setUsbInit(int s);
static GThreadFunc usb_read_monitor(gpointer data) ;

static void *register_usb_cb(void *cb) 
{
    if (usb_init() == 0 && usb_enumerate(VID, PID) == 0) {
        setUsbInit(1);        
    }
    else {
        printf("%s: failed!\n", __func__);
        return NULL;
    }

    void *g = (void *)g_thread_new("usb-fw_upgread-test", usb_read_monitor, cb);
    usleep(1000);
    return g;
}

static GThreadFunc usb_read_monitor(gpointer data) 
{
    char buf[64] = {0};
    FuncPtrIII cb = (FuncPtrIII)data;

    do {
        if (usb_read(buf) < 0) {
            usb_close();
            break;
        }
        cb(buf);
    }
    while (1);

    g_thread_exit(NULL);
    return NULL;
}

static void sem_post_t(void) 
{	
    printf("[%s]: **** ENTER **** \n",__func__);
#ifdef __linux
    sem_post(&sem);
#else
    ReleaseSemaphore(sem, 1, NULL);
#endif
}

static void sem_wait_t(void) 
{
    printf("[%s]: enter\n",__func__);
#ifdef __linux
    sem_wait(&sem);
#else
    WaitForSingleObject(sem, INFINITE);
#endif
    printf("[%s]: exit\n",__func__);
}

static void usb_msg_resp_callback(char *buf)
{
    printf("\n **************** RESP **************** \n");
    for (int i = 0; i < 64; i++) { printf("%02x", buf[i]); }
    printf("\n");

    if (memcmp(buf, "?##", 3) == 0) {
        switch (step_flag) {
            case REQ_INITIAL:
                if (buf[4] == 0x11 && buf[34] == 0x28 && buf[35] == 0x01) {
                    step_flag = RES_SUCCESS;
                } 
                else {
                    step_flag = RES_ERROR;
                }
                break;

            case REQ_UPGRADE_FW:
                if (buf[4] == 0x1a && buf[9] == 0x08 && buf[10] == 0x09) {
                    step_flag = REQ_CHECK;
                }
                else if (buf[4] == 0x02) {
                    step_flag = RES_SUCCESS;
                }
                else {
                    step_flag = RES_ERROR;
                }
                break;

            case REQ_ERASE:
            case REQ_CHECK:
                if (buf[4] == 0x02) {
                    step_flag = RES_SUCCESS;
                }
                else {
                    step_flag = RES_ERROR;
                }
                break;
        }
        sem_post_t();
    }
}

static unsigned long readprotobufint(unsigned char **ptr)
{
    unsigned long result = (**ptr & 0x7F);
    if (**ptr & 0x80) {
        (*ptr)++;
        result += (**ptr & 0x7F) * 128;
        if (**ptr & 0x80) {
            (*ptr)++;
            result += (**ptr & 0x7F) * 128 * 128;
            if (**ptr & 0x80) {
                (*ptr)++;
                result += (**ptr & 0x7F) * 128 * 128 * 128;
                if (**ptr & 0x80) {
                    (*ptr)++;
                    result += (**ptr & 0x7F) * 128 * 128 * 128 * 128;
                }
            }
        }
    }
    (*ptr)++;
    return result;
}

static int encode_bin_size(unsigned char *buf, unsigned long size) 
{
    long t, t1, t2, t3, t4, t5;
    unsigned char *pp = buf;
    int len = 0;

    if (size >= 128*128*128) { // 2097152= 2048 KB
        t = size / (128*128*128);
        t1 = size % (128*128*128);
        t2 = t1 / (128*128);
        t3 = t1 % (128*128);
        t4 = t3 / 128;
        t5 = t3 % 128;

        buf[0] = 0x80 | t5;
        buf[1] = 0x80 | t4;
        buf[2] = 0x80 | t2;
        buf[3] = t;
        len = 4;
    }
    else if (size >= 128*128) { // 16384 = 16 KB
        t = size / (128*128);
        t1 = size % (128*128);
        t2 = t1 / 128;
        t3 = t1 % 128;

        buf[0] = 0x80 | t3;
        buf[1] = 0x80 | t2;
        buf[2] = t;
        len = 3;
    }
    else if (size >= 128) { // 256 B
        t = size / 128;
        t1 = size % 128;

        buf[0] = 0x80 | t1; 
        buf[1] = t;
        len = 2;
    }

    unsigned long result = readprotobufint(&pp);
    if (result != size) {
        printf("%s: encode (%ld) err!\n", __func__, size);
        return 0;
    }

    printf("%s: size(%ld) [0x%02x,0x%02x,0x%02x,0x%02x] len:%d\n", __func__, size, buf[0], buf[1], buf[2], buf[3], len);
    return len;
}

void HexStrToByte(const unsigned char *s, unsigned char *d, int len)
{
    int h, l;

    for (int i = 0; i < len; i += 2) {
        h = s[i];
        l = s[i + 1];

        if (h > 0x39)
            h -= 0x57;
        else
            h -= 0x30;

        if (l > 0x39)
            l -= 0x57;
        else
            l -= 0x30;

        d[i / 2] = (h << 4) | l;
    }
}

static int device_reboot(void) 
{
    unsigned char *cmd = "0074000000020802";
    unsigned char buf[64] = {0};

    HexStrToByte(cmd, buf, strlen(cmd));

    if (usb_write(buf) < 0) {
        goto err;
    }

    return 0;

err:
    return -1;
}

static int firmware_upgrade(char *filename)
{	
    FILE *fp;
    unsigned char buf[READ_BUFF] = {0};
    unsigned int c = 0;
    unsigned int bin_size = 0;
    unsigned int msg_id = 0;
    const char *binfile = (const char *)((strlen(filename) == 0) ? BIN_FILE_PATH : filename);

    if (filename) {
        printf("[%s]:%s\n",__func__, filename);
    }

    // get bin size.
    fp = fopen(binfile,"rb+");
    if (fp == NULL) {
        printf("[%s]: Cannot open: %s\n",__func__, binfile);
        goto err;
    }
    while((c = fread(buf, sizeof(unsigned char), READ_BUFF, fp)) > 0) {
        bin_size += c;
        if (bin_size <= 256) { for (int i = 0; i < 64; i++) { printf("%02x", buf[i]); } }
    }
    fclose(fp);
    printf("\n");

    //NO.1 Initialize message msg_id == 0
    memset(buf, 0, sizeof(buf));
    buf[0] = '?';
    buf[1] = '#';
    buf[2] = '#';
    buf[3] = 0;
    buf[4] = 0x0;

    step_flag = REQ_INITIAL;
    if (usb_write(buf) < 0) {
        goto err;
    }
    sem_wait_t();
    if (step_flag == RES_ERROR) {
        printf("Device not in bootloader mode..\n");
        goto err;
    }
    printf("[%s:%s] Initialize resp ok(%d)\n",__FILE__,__func__, bin_size);

    //NO.2 FirmwareErase message msg_id == 0x6
    memset(buf, 0, sizeof(buf));
    buf[0] = '?';
    buf[1] = '#';
    buf[2] = '#';
    buf[3] = 0;
    buf[4] = 0x06;

    step_flag = REQ_ERASE;
    if (usb_write(buf) < 0) {
        goto err;
    }
    sem_wait_t();
    if (step_flag == RES_ERROR) {
        printf("Firmware erase ERROR!\n");
        goto err;
    }
    printf("[%s:%s] Firmware erase resp ok!\n",__FILE__,__func__);

    //NO.3 Firmware upgrade start msg_id == 0x7
    memset(buf, 0, sizeof(buf));
    buf[0] = '?';
    buf[1] = '#';
    buf[2] = '#';
    buf[3] = 0;
    buf[4] = 0x07;
    buf[9] = 0x0a;

    int len = encode_bin_size(buf + 10, bin_size);
    fp = fopen(binfile,"rb+");
    if((c = fread(buf + 10 + len, sizeof(unsigned char), READ_BUFF -10 -len, fp)) > 0) {
        step_flag = REQ_UPGRADE_FW;
        if (usb_write(buf) < 0) {
            goto err;
        }
    }
    while((c = fread(buf + 1, sizeof(unsigned char), READ_BUFF -1, fp)) > 0) {	
        if (usb_write(buf) < 0) {
            goto err;
        }
    }
    fclose(fp);

    sem_wait_t();
    if (step_flag == RES_ERROR) {
        printf("Firmware Upgrade error !\n");
        goto err;
    }
    printf("[%s:%s] Firmware Upgrade resp completed\n",__FILE__,__func__);

    //NO.4 Firmware upgrade check msg_id == 0x1b
    if (step_flag == REQ_CHECK) {
        memset(buf, 0, sizeof(buf));
        buf[0] = '?';
        buf[1] = '#';
        buf[2] = '#';
        buf[3] = 0;
        buf[4] = 0x1b;

        if (usb_write(buf) < 0) {
            goto err;
        }

        sem_wait_t();
        if (step_flag == RES_ERROR) {
            printf("Firmware Upgrade error !\n");
            goto err;
        }
        printf("[%s:%s] Firmware Upgrade resp check done!\n",__FILE__,__func__);
    }
    return 0;

err:
    return -1;
}

static int goto_erase_firmware(void) 
{
    unsigned char buf[READ_BUFF];

    //NO.1 Initialize message msg_id == 0
    memset(buf, 0, sizeof(buf));
    buf[0] = '?';
    buf[1] = '#';
    buf[2] = '#';
    buf[3] = 0;
    buf[4] = 0x00;

    step_flag = REQ_INITIAL;
    if (usb_write(buf) < 0) {
        goto err;
    }
    printf("[%s:%s] Initialize resp success!\n",__FILE__,__func__);

    sem_wait_t();
    if (step_flag == RES_ERROR) {
        printf("Device not in bootloader mode..\n");
        goto err;
    }

    //NO.2 FirmwareErase message msg_id == 0x6
    memset(buf, 0, sizeof(buf));
    buf[0] = '?';
    buf[1] = '#';
    buf[2] = '#';
    buf[3] = 0;
    buf[4] = 0x06;
    buf[9] = 0x98; // erase firmware only
    buf[10] = 0x1c;

    step_flag = REQ_ERASE;
    if (usb_write(buf) < 0) {
        goto err;
    }

    sem_wait_t();
    if (step_flag == RES_ERROR) {
        printf("Firmware erase FAILED!\n");
        goto err;
    }

    printf("[%s:%s] Firmware erase resp success. device reboot..!\n",__FILE__,__func__);
    return 0;

err:
    return -1;
}

int usb_close(void);
int main(int argc,char *argv[])
{	
    int opt;
    int erase = 0;
    int upgrade = 0;
    static char *optstring = "a:f:";
    char buf[256] = {0};

    while ((opt = getopt(argc, argv, optstring)) != -1) {		
        switch (opt) {
            case 'a':
                if (strcmp(optarg, "erase") == 0) {
                    erase = 1;
                }
                break;
            case 'f':
                if (optarg != NULL) {
                    upgrade = 1;
                    strcpy(buf, optarg);
                }
                break;
        }
    }

    if (!register_usb_cb(usb_msg_resp_callback)) {
        return -1;
    }

#ifndef __linux
    sem = CreateSemaphore(NULL, 0, 1, NULL);
#endif

    if (erase) {
        goto_erase_firmware();
    }

    if (upgrade) {
        firmware_upgrade(buf);
    }

    usb_close();
    printf("[%s] exit.\n",__func__);
    return 0;
}



























