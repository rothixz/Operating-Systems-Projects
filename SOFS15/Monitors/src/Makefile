CC = gcc
CFLAGS = -Wall

SUFFIX = $(shell getconf LONG_BIT)

LIBNAME = MonGameOfRope

LIBS =
LIBS += -l$(LIBNAME)
LIBS += -l$(LIBNAME)_bin_$(SUFFIX)
LIBS += -l$(LIBNAME)_aux

LFLAGS = -L ../lib $(LIBS) -lm -lpthread

TARGET = probMonGameOfRope

AUX_OBJS =  monGameOfRopeDS.o logging.o

OBJS =
#OBJS +=  monGameOfRopeRF.o
OBJS +=  monGameOfRopeCH.o
OBJS +=  monGameOfRopeCT.o

.PHONY: all auxlib lib exe clean cleanall

all:		lib auxlib exe clean

lib:		$(OBJS)
			ar r ../lib/lib$(LIBNAME).a $^

auxlib:    $(AUX_OBJS)
			ar r ../lib/lib$(LIBNAME)_aux.a $^

exe:	$(TARGET).o lib auxlib
			$(CC) -o ../run/$(TARGET) $< $(LFLAGS)

clean:
			rm -f *.o ../lib/lib$(LIBNAME).a ../lib/lib$(LIBNAME)_aux.a

cleanall:   clean
			rm -f ../run/$(TARGET)
