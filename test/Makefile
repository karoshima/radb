################################################################

TARGET = radb_test
OBJS =   sut.o
LIBS =   ../src/radb.a

CFLAGS += -g -Wall

all: $(TARGET)

clean:
	rm -f $(OBJS) *~ .depend

################################################################

SRCS =   $(OBJS:.o=.c)

depend .depend: $(SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -M $(SRCS) > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif

################################################################

$(TARGET): $(OBJS) $(LIBS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(LIBS)::
	cd ../src; $(MAKE) test

$(OBJS): $(SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

################################################################

.SUFFIXES: .c .h .i .s .o .a

.c.i:
	$(CC) $(CPPFLAGS) $(CFLAGS) -E $< -o $@

.c.s:
	$(CC) $(CPPFLAGS) $(CFLAGS) -S $<
