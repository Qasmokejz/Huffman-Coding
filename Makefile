CC=clang
EXEC1=huff

CFLAGS=-Wall -Wextra -Wstrict-prototypes -Werror -pedantic
CFLAGS-debug=-Wall -Wextra -Wstrict-prototypes -Werror -pedantic -g -gdwarf-4

OBJS1=huff.o bitwriter.o node.o pq.o
btst=bwtest.o bitwriter.o
ntst=nodetest.o node.o
ptst=pqtest.o pq.o node.o

LIBS1=io-$(shell uname -m).a

all: $(EXEC1) nodetest bwtest pqtest

$(EXEC1): $(OBJS1) $(LIBS1)
	$(CC) $(CFLAGS) -o $@ $^

bwtest: $(btst) $(LIBS1)
	$(CC) $(CFLAGS-debug) -o $@ $^

nodetest: $(ntst) $(LIBS1)
	$(CC) $(CFLAGS-debug) -o $@ $^
pqtest: $(ptst) $(LIBS1)
	$(CC) $(CFLAGS-debug) -o $@ $^

bitwriter.o: bitwriter.c bitwriter.h
	$(CC) $(CFLAGS) -c $< -o $@
node.o: node.c node.h
	$(CC) $(CFLAGS) -c $< -o $@
pq.o: pq.c pq.h
	$(CC) $(CFLAGS) -c $< -o $@

bwtest.o: bwtest.c $(LIBS1)
	$(CC) $(CFLAGS) -c $< -o $@
nodetest.o: nodetest.c $(LIBS1)
	$(CC) $(CFLAGS) -c $< -o $@
pqtest.o: pqtest.c $(LIBS1)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -f huff bwtest nodetest pqtest *.o

format:
	clang-format -i --style=file *.[ch]


