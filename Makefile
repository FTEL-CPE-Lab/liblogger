CC = cc

CFLAGS  = -std=c99 -O2 -Wall -I/mnt/local/include -DHAVE_JANSSON -DTHREAD_ENABLE -fsanitize=address
LDFLAGS = -ljansson -L/mnt/local/lib -lpthread

NAME = liblogger

UNAME_S := $(shell uname -s)

INCDIR := /mnt/local/include
LIBDIR  = /mnt/local/lib

ifeq ($(UNAME_S),Linux)
$(NAME).so: logger.h
	$(CC) -shared -o $@ $(CFLAGS) $(LDFLAGS)
endif
ifeq ($(UNAME_S),Darwin)
$(NAME).dylib:
	$(CC) -c -dynamiclib -o $@ $(CFLAGS) $(LDFLAGS)
endif

.PHONY: install
install: 
	cp logger.h $(INCDIR)
ifeq ($(UNAME_S),Linux)
	cp logger.h $(INCDIR)
	cp $(NAME).so $(LIBDIR)
endif
ifeq ($(UNAME_S),Darwin)
	cp logger.h $(INCDIR)
	cp $(NAME).dylib $(LIBDIR)
endif

.PHONY: uninstall
uninstall:
	rm -f $(INCDIR)/logger.h
ifeq ($(UNAME_S),Linux)
	rm -f $(INCDIR)/$(NAME).so
endif
ifeq ($(UNAME_S),Darwin)
	rm -f $(INCDIR)/$(NAME).dylib
endif

example:
	$(CC) -o $@ example.c $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(NAME).dylib
	rm -f $(NAME).so
	rm -f example
