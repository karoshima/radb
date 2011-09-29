################################################################

TARGET = radb.a
OBJS =   radb_bit.o

MKOPT =  mkopt.h
SRCS =   $(OBJS:.o=.c) $(MKOPT)

CFLAGS +=   -g -Wall
CPPFLAGS += 

all: $(TARGET)

clean:
	rm -f $(OBJS) *~ .depend

test:
	echo "#define TEST" > $(MKOPT)
	$(MAKE) clean
	$(MAKE) all
	$(MAKE) clean
	$(MAKE) $(MKOPT)

depend .depend: $(SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -M $(SRCS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

$(TARGET): $(OBJS)
	rm -f $(TARGET)
	ar ruvs $(TARGET) $(OBJS)

$(MKOPT):
	cp /dev/null $(MKOPT)
