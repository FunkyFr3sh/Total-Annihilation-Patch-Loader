-include config.mk

TARGET       = dplayx.dll

LDFLAGS      = -Wl,--enable-stdcall-fixup -s -shared -static
CFLAGS       = -std=gnu99 -masm=intel -O2 -march=pentium4 -Iinih -Wall
CFLAGS      += -DINI_CALL_HANDLER_ON_NEW_SECTION=1 -DINI_ALLOW_MULTILINE=0 -DINI_MAX_LINE=1544 -DINI_STOP_ON_FIRST_ERROR=1
LIBS         =

CC           = i686-w64-mingw32-gcc
CXX          = i686-w64-mingw32-g++
STRIP       ?= i686-w64-mingw32-strip
WINDRES     ?= i686-w64-mingw32-windres

OBJS         = \
				res/res.o \
				dllmain.o \
				patches.o \
				inih/ini.o

.PHONY: default clean
default: $(TARGET)

%.o: %.rc
	$(WINDRES) -J rc $< $@

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ exports.def $(LIBS)

clean:
	$(RM) $(OBJS) $(TARGET)
