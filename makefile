-include config.mk

TARGET       = dplayx
LDFLAGS      = -mdll -Wl,--enable-stdcall-fixup -Wl,--strip-all -Wl,--exclude-all-symbols
CFLAGS       = -std=gnu99 -masm=intel -O2 -march=pentium4 -Iinih/ -Wall
WINDRES     ?= windres

OBJS         = \
				res/res.o \
				dllmain.o \
				patches.o \
				inih/ini.o

.PHONY: default
default: $(TARGET).dll

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.rc
	$(WINDRES) -J rc $< $@

$(TARGET).dll: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(TARGET).def -lmsvcrt

clean:
	$(RM) $(OBJS) $(TARGET).dll
