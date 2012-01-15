SRCDIR=src
BINDIR=bin
LIBDIR=lib
CC=gcc
AR=ar

CFLAGS=-O2 -I$(SRCDIR) -g -std=c99 -Wall
LDFLAGS=-lusb

all: $(LIBDIR)/libvk8055.a test-software

test-software: $(BINDIR)/k8055-test-out $(BINDIR)/k8055-test-in

$(BINDIR)/k8055-test-out:	$(SRCDIR)/test/testout.c $(BINDIR)/k8055.o
	$(CC) $(CFLAGS) $(SRCDIR)/test/testout.c $(BINDIR)/k8055.o $(LDFLAGS) -o $(BINDIR)/k8055-test-out

$(BINDIR)/k8055-test-in:	$(SRCDIR)/test/testin.c $(BINDIR)/k8055.o
	$(CC) $(CFLAGS) $(SRCDIR)/test/testin.c $(BINDIR)/k8055.o $(LDFLAGS) -o $(BINDIR)/k8055-test-in

$(LIBDIR)/libvk8055.a: $(BINDIR)/k8055.o
	$(AR) -rcs -o $(LIBDIR)/libvk8055.a $(BINDIR)/k8055.o

$(BINDIR)/k8055.o: $(SRCDIR)/k8055.c
	$(CC) $(CFLAGS) -c $(SRCDIR)/k8055.c -o $(BINDIR)/k8055.o

clean:
	rm $(BINDIR)/*
	rm $(LIBDIR)/*.a
