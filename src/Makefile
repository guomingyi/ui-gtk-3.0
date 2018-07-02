OUT := test
ARCH := amd64

CC := x86_64-linux-gnu-gcc # linxu gcc
CC_WIN := x86_64-w64-mingw32-gcc # windows gcc

SRC := \
  main.c \
  listdevs.c

# libusb.
LIBNAME  := -lusb-1.0
ifeq ($(strip $(BUILD)), win-amd64)
  LIBLOAD  := -L ./libs/windows/libusb-1.0.22/MinGW64/static
  LIBINC := -I ./libs/windows/libusb-1.0.22/include/libusb-1.0
else 
# linux-amd64
  LIBLOAD  := -L ./libs/linux/lib
  LIBINC := -I ./libs/linux/include/libusb-1.0
endif

# gtk+3.0
CFLAGS := `pkg-config --cflags --libs gtk+-3.0`
CFLAGS += $(LIBNAME) $(LIBLOAD) $(LIBINC)

all: clean windows linux

windows:
	$(CC_WIN)  $(SRC) -o $(OUT)-windows-$(ARCH).exe $(CFLAGS)
	@mv $(OUT)-windows-$(ARCH).exe build && tree build|grep $(OUT)

linux:
	$(CC) $(SRC) -o $(OUT)-linux-$(ARCH) $(CFLAGS)
	@mv $(OUT)-linux-$(ARCH) build && tree build|grep $(OUT)

clean:
	@rm -rf build/$(OUT)*


