CFLAGS = -Wall -I. -O

LDLIBS =  

SRC = \
  cmdline.c \
  adnpfunc.c \
  dnpsetup.c

OBJ = $(SRC:%.c=%.o)

dnpsetup:$(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LDLIBS)

cmdline.c: cmdline.cli
	clig $<

clean:
	rm -f *.o *~ dnpsetup

