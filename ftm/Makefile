OUT := test

#ARCH ?= amd64
ARCH ?= 386

# linxu gcc
CC := x86_64-linux-gnu-gcc

# windows gcc
ifeq ($(strip $(ARCH)), amd64)
  MINGW := MinGW64
  CC_WIN := x86_64-w64-mingw32-gcc
  LIB_WIN := libs/windows/gtk+-bundle_3.10.4-20131202_win64
else
  MINGW := MinGW32
  CC_WIN := i686-w64-mingw32-gcc
  LIB_WIN := libs/windows/gtk+-bundle_3.6.4-20130921_win32
endif

##############################################################
SRC := \
  listdevs.c \
  usb.c \
  usb-hid-api.c

SRC_BTN := \
  test_pcba_btn.c

SRC_BTN_OLED := \
  test_pcba_btn_oled.c

SRC_BTN_OLED_FW := \
  test_pcba_btn_oled_fw.c

SRC_FW_PROGRAM := \
  download/main.c \
  download/download_callback.c \
  download/factory_download_ui.c \
  download/factory_main_ui.c
##############################################################
# libusb.
LIBNAME  := -lusb-1.0
ifeq ($(strip $(BUILD)), linux)
  LIBLOAD  := -L ./libs/linux/lib
  LIBINC := -I ./libs/linux/include/libusb-1.0
else 
  LIBLOAD  := -L ./libs/windows/libusb-1.0.22/$(MINGW)/static
  LIBINC := -I ./libs/windows/libusb-1.0.22/include/libusb-1.0
  exec := .exe
endif

# gtk+3.0
CFLAGS := `pkg-config --cflags --libs gtk+-3.0 gthread-2.0 `
CFLAGS += $(LIBNAME) $(LIBLOAD) $(LIBINC) -Wfatal-errors -g -Wall -Wunused-variable
##############################################################

all: clean windows linux
linux: copy_depends btn btn_oled btn_oled_fw_erase fw_program
windows: copy_depends_win btn-win btn_oled-win btn_oled_fw_erase-win fw_program-win

btn:
	$(CC) $(SRC_BTN) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_btn-linux-$(ARCH)
	tree build|grep $(OUT)

btn_oled:
	$(CC) $(SRC_BTN_OLED) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_btn_oled-linux-$(ARCH)
	tree build|grep $(OUT)

btn_oled_fw_erase:
	$(CC) $(SRC_BTN_OLED_FW) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_btn_oled_fw_erase-linux-$(ARCH)
	tree build|grep $(OUT)

fw_program:
	$(CC) $(SRC_FW_PROGRAM) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_fw_program-linux-$(ARCH)
	@cp -f download/factory_download.glade build
	tree build|grep $(OUT)

btn-win:
	$(CC_WIN) $(SRC_BTN) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_btn-windows-$(ARCH).exe
	tree build|grep $(OUT)

btn_oled-win:
	$(CC_WIN) $(SRC_BTN_OLED) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_btn_oled-windows-$(ARCH).exe
	tree build|grep $(OUT)

btn_oled_fw_erase-win:
	$(CC_WIN) "-m32" $(SRC_BTN_OLED_FW) $(SRC) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/$(OUT)_btn_oled_fw_erase-windows-$(ARCH).exe
	tree build|grep $(OUT)

fw_program-win: copy_depends_win
	$(CC_WIN) $(SRC_FW_PROGRAM) -o build/$(OUT) $(CFLAGS)
	@mv build/$(OUT) build/bin/$(OUT)_fw_program-windows-$(ARCH).exe
	tree build|grep $(OUT)

copy_depends_win:
	@[ -d build/bin ] || mkdir -p build/bin
	@[ -d build/share/glib-2.0 ] || mkdir -p build/share/glib-2.0
	cp -f download/factory_download.glade build/bin
	cp -f download/download.bat build/bin
	cp -a $(LIB_WIN)/share/glib-2.0/schemas build/share/glib-2.0
	cp -f $(LIB_WIN)/bin/*.dll build/bin && chmod 644 build/bin/*.dll
	cp -f $(LIB_WIN)/bin/*.dll build/ && chmod 644 build/*.dll
	cp -f builder.ui build

copy_depends:
	@[ -d build/bin ] || mkdir -p build/bin
	cp -f download/factory_download.glade build/bin
	cp -f download/download.sh build/bin
	cp -f builder.ui build

clean:
	@rm -rf build/$(OUT)* build/bin build/*.dll


