-include config.mk

TARGET       = dplayx
LDFLAGS      = -mdll -Wl,--enable-stdcall-fixup -Wl,--strip-all -Wl,--exclude-all-symbols
CFLAGS       = -std=gnu99 -masm=intel -O2 -march=pentium4 -Iinih/ -Wall
CFLAGS      += -DINI_CALL_HANDLER_ON_NEW_SECTION=1 -DINI_ALLOW_MULTILINE=0 -DINI_MAX_LINE=1544 -DINI_STOP_ON_FIRST_ERROR=1
LIBS         = -lmsvcrt

OBJS         = \
				res/res.o \
				dllmain.o \
				patches.o \
				inih/ini.o

WINDRES     ?= windres

.PHONY: default
default: $(TARGET).dll

%.o: %.rc
	$(WINDRES) -J rc $< $@

$(TARGET).dll: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(TARGET).def $(LIBS)

clean:
	$(RM) $(OBJS) $(TARGET).dll
