CC=gcc
CFLAGS=-I.

ODIR=obj
IDIR=../include
BDIR=../bin
LDIR=../lib

_DEPS=banksystem.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ=banksysfunctions.o BankSystem.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ));

$(ODIR)/banksysfunctions.o: banksysfunctions.c
	$(CC) -o $@ -c $^ -fPIC $(CFLAGS)

$(ODIR)/BankServerUtilities.o: BankServerUtilities.c
	$(CC) -o $@ -c $^ -fPIC $(CFLAGS)

$(ODIR)/Bankserver.o: Bankserver.c 
	$(CC) -o $@ -c $^

$(LDIR)/libbanksystem.so: $(ODIR)/banksysfunctions.o 
	$(CC) -shared -fPIC -o $@ $^

$(LDIR)/libbankserver.so: $(ODIR)/BankServerUtilities.o
	$(CC) -shared -fPIC -o $@ $^
		
$(ODIR)/BankSystem.o: BankSystem.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BDIR)/BankSystem.out: $(ODIR)/BankSystem.o  $(LDIR)/libbanksystem.so
	$(CC) -o $@ $^ $(CFLAGS) $(LDIR)/libbanksystem.so -pthread

$(BDIR)/BankSystem1.out: $(ODIR)/BankSystem.o  $(LDIR)/libbanksystem.so
	$(CC) -o $@ $^ $(CFLAGS) $(LDIR)/libbanksystem.so -pthread

$(BDIR)/BankServer.out: $(ODIR)/Bankserver.o $(LDIR)/libbankserver.so
	$(CC) -o $@ $^ $(CFLAGS) $(LDIR)/libbankserver.so -pthread

.PHONY: clean

# removes object files and temporary files created with ~ at the end of file name in current i.e. source and include directory
clean: 
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
	rm -f $(BDIR)/*.out $(LDIR)/*.so
	
.PHONY: all
all: $(LDIR)/libbanksystem.so $(BDIR)/BankSystem.out $(BDIR)/BankServer.out $(BDIR)/BankSystem1.out
