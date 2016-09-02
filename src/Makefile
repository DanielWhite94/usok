CC = clang
CFLAGS = -O0 -ggdb3 -DKeyU=XK_Up -DKeyD=XK_Down -DKeyL=XK_Left -DKeyR=XK_Right
LFLAGS = -lX11 -lm

OBJS = usok.o

ALL: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o ./usok $(LFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
