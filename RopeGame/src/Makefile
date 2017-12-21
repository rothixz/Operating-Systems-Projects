CC = gcc
CFLAGS = -Wall

SUFFIX = $(shell getconf LONG_BIT)

REFEREE = semSharedMemReferee
COACH = semSharedMemCoach
CONTESTANT = semSharedMemContestant
MAIN = probSemSharedMemGameOfRope

OBJS = sharedMemory.o semaphore.o logging.o

.PHONY: all ct ct_ch all_bin \
	main referee coach contestant \
	referee_bin coach_bin contestant_bin \
	clean cleanall

all:		contestant     coach     referee     main clean
ct:		contestant     coach_bin referee_bin main clean
ct_ch:		contestant     coach     referee_bin main clean
all_bin:	contestant_bin coach_bin referee_bin main clean

referee:	$(REFEREE).o $(OBJS)
	$(CC) -o ../run/$@ $^ -lm

coach:		$(COACH).o $(OBJS)
	$(CC) -o ../run/$@ $^

contestant:	$(CONTESTANT).o $(OBJS)
	$(CC) -o ../run/$@ $^ -lm

main:		$(MAIN).o $(OBJS)
	$(CC) -o ../run/$(MAIN) $^ -lm

referee_bin:
	cp ../run/referee_bin_$(SUFFIX) ../run/referee

coach_bin:
	cp ../run/coach_bin_$(SUFFIX) ../run/coach

contestant_bin:
	cp ../run/contestant_bin_$(SUFFIX) ../run/contestant

clean:
	rm -f *.o

cleanall:	clean
	rm -f ../run/$(MAIN) ../run/referee ../run/coach ../run/contestant

