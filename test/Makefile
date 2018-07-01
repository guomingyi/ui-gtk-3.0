OUT := test.exe

CC := x86_64-w64-mingw32-gcc
CFLAGS := `pkg-config --cflags --libs gtk+-3.0`
CONFIG = ~/tools/gtk+-bundle_3.10.4-20131202_win64/lib/pkgconfig

SRC := main.c

all:
	@export PKG_CONFIG_PATH=$(CONFIG)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)
	@mv $(OUT) build

clean:
	@rm -rf build/$(OUT)


