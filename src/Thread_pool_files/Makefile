CFLAGS = -g -Wall
EXECUTABLE = picalc

picalc: picalc.o pool.o cqueue.o spinlock.o
	gcc -o ${EXECUTABLE} ${CFLAGS} picalc.o pool.o cqueue.o spinlock.o -lm

picalc.o: picalc.c rc.h pool.h
	gcc -c ${CFLAGS} picalc.c

pool.o: pool.c pool.h rc.h cqueue.h
	gcc -c ${CFLAGS} -pthread pool.c

cqueue.o: cqueue.c cqueue.h spinlock.h
	gcc -c ${CFLAGS} cqueue.c

spinlock.o: spinlock.c spinlock.h
	gcc -c ${CFLAGS} spinlock.c 

clean:
	rm -f *.o picalc
	rm -f core*