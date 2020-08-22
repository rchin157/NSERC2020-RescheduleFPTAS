fptas: main.o annotate.o dp.o enumerate.o util.o
	gcc -Wall -std=c99 -lm main.o annotate.o dp.o enumerate.o util.o -o fptas

main.o: main.c main.h
	gcc -Wall -std=c99 -c main.c

annotate.o: annotate.c annotate.h
	gcc -Wall -std=c99 -c annotate.c

dp.o: dp.c dp.h
	gcc -Wall -std=c99 -c dp.c

enumerate.o: enumerate.c enumerate.h
	gcc -Wall -std=c99 -c enumerate.c

util.o: util.c util.h
	gcc -Wall -std=c99 -c util.c

clean:
	rm -f *.out *.o

