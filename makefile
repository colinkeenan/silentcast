TARGET=silentcast
OBJS=    main.o SC_X11_get_active_window.o SC_conf_widgets.o SC_temptoanim.o
BINDIR=scpkg
 
CC=gcc
DEBUG=-g
OPT=-O0
WARN=-Wall
VERS=-std=gnu99
vpath %.c lib:src
 
PTHREAD=-pthread
 
CCFLAGS=$(VERS) $(DEBUG) $(OPT) $(WARN) $(PTHREAD) -pipe
 
GTKX11LIB = `pkg-config --cflags --libs x11 gtk+-x11-3.0`
SCLIB = -I./lib
 
# linker
LD=gcc
LDFLAGS=$(PTHREAD) $(GTKX11LIB) -export-dynamic -lm
 
all: $(OBJS)
	$(LD) -o $(BINDIR)/$(TARGET) $(OBJS) $(LDFLAGS)
    
%.o: %.c
	$(CC) -c $(CCFLAGS) $< $(GTKX11LIB) -o $@ $(SCLIB)
    
clean:
	rm -f *.o $(BINDIR)/$(TARGET)
