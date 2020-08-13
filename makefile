exactDP: main.o annotate.o dp.o enumerate.o automate.o interval.o util.o
	gcc -Wall -std=c99 -lm main.o annotate.o dp.o enumerate.o automate.o interval.o util.o -o exactDP

arrTest: arrTest.c util.o
	gcc -Wall -pedantic -std=c99 -lm -o arrTest arrTest.c util.o
	
main.o: main.c main.h
	gcc -Wall -std=c99 -c main.c

annotate.o: annotate.c annotate.h
	gcc -Wall -std=c99 -c annotate.c

dp.o: dp.c dp.h
	gcc -Wall -std=c99 -c dp.c

enumerate.o: enumerate.c enumerate.h
	gcc -Wall -std=c99 -c enumerate.c

automate.o: automate.c automate.h
	gcc -Wall -std=c99 -c automate.c

interval.o: interval.c interval.h
	gcc -Wall -std=c99 -c interval.c

util.o: util.c util.h
	gcc -Wall -std=c99 -c util.c

clean:
	rm -f *.out *.o

