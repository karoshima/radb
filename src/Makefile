################################################################

TARGET = radb.a
OBJS =   radb_bit.o

MKOPT =  mkopt.h
SRCS =   $(OBJS:.o=.c) $(MKOPT)

CFLAGS +=   -g -Wall
CPPFLAGS += 

all: $(TARGET)

clean:
	rm -f $(OBJS) $(MKOPT) *~ .depend

test:
	$(MAKE) clean
	echo "#define TEST" > $(MKOPT)
	$(MAKE) all
	$(MAKE) $(MKOPT)

################################################################

depend .depend: $(SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -M $(SRCS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

################################################################

$(TARGET): $(OBJS)
	rm -f $(TARGET)
	ar ruvs $@ $(OBJS)

$(MKOPT):
	cp /dev/null $@

################################################################

.SUFFIXES: .c .h .i .s .o .a

.c.i:
	$(CC) $(CPPFLAGS) $(CFLAGS) -E $< -o $@

.c.s:
	$(CC) $(CPPFLAGS) $(CFLAGS) -S $<
