CFLAGS = -g
EXECUTABLE = qmain 

qmain: qmain.o cqueue.o spinlock.o
	gcc -o ${EXECUTABLE} ${CFLAGS} -pthread qmain.o cqueue.o spinlock.o

qmain.o: qmain.c cqueue.h
	gcc -c ${CFLAGS} qmain.c 

cqueue.o: cqueue.c cqueue.h spinlock.h
	gcc -c ${CFLAGS} cqueue.c

spinlock.o: spinlock.c spinlock.h
	gcc -c ${CFLAGS} spinlock.c 

clean:
	rm -f *.o qmain
	rm -f core*