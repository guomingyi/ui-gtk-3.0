OUT := test

# linxu gcc
CC := x86_64-linux-gnu-gcc

# windows gcc
CC_WIN := x86_64-w64-mingw32-gcc

ARCH := amd64

CFLAGS := `pkg-config --cflags --libs gtk+-3.0`

SRC := main.c


all: clean windows linux

windows:
	$(CC_WIN)  $(SRC) -o $(OUT)-windows-$(ARCH).exe $(CFLAGS)
	@mv $(OUT)-windows-$(ARCH).exe build && tree build|grep $(OUT)

linux:
	$(CC) $(SRC) -o $(OUT)-linux-$(ARCH) $(CFLAGS)
	@mv $(OUT)-linux-$(ARCH) build && tree build|grep $(OUT)

clean:
	@rm -rf build/$(OUT)*


