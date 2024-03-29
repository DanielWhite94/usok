#include <stdio.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#define I int

C[]={255,16245355,5057287,7697781,1,6052956};
long long IM[][6]={
 [1]={4,0,0,0,0,0},
 [2]={5,0,0,0,0x183c3c180000ll,0},
 [3]={5,0,0,0,0,0},
 [7]={5,3,1,2,0x447c7dfe01297d39ll,0x44006d017d7c0000ll},
 [10]={5,4,0,0,0x42240000244200ll,0x183c3c180000ll},
 [11]={5,4,0,0,0x42241818244200ll,0},
 [15]={5,3,1,2,0x447c7dfe01297d39ll,0x44006d017d7c0000ll},
};

Display *D;Window W;GC G; // xlib stuff
L[512][512]={1}; // level
P, Q; // player's X and Y

void imageDraw(long long *i, x, y) {
	I z, c;
	for(z=0;z<64;++z)
		XSetForeground(D, G, C[i[((i[5]>>z)%2<<1)|(i[4]>>z)%2]]),
		XFillRectangle(D, W, G, x+z%8*2, y+z/8*2, 2, 2);
}

main(y) {
	// load level
	I c, x=y=256;
	while((c=getchar())!=EOF)
		switch(c) {
			case 35: L[y][x++]=1; break;
			case 64: L[y][x]=7; P=x++, Q=y; break;
			case 43: L[y][x]=15; P=x++, Q=y; break;
			case 36: L[y][x++]=2; break;
			case 42: L[y][x++]=10; break;
			case 46: L[y][x++]=11; break;
			case 32: L[y][x++]=3; break;
			case 10: ++y; x=256; break;
		}

	// drawing initialization
	D=XOpenDisplay(0);
	XSelectInput(D, W=XCreateSimpleWindow(D,RootWindow(D,0),0,0,640,480,0,0,0), KeyPressMask);
	XMapWindow(D,W);
	G=XCreateGC(D,W,0,0);

	// main loop
	for(;;) {
		// draw base map
		for(y=-15; y<=16; ++y)
			for(x=-20; x<=21; ++x)
				imageDraw(IM[L[y+256][x+256]], 16*(x+20), 16*(y+15));

		// check for key press
		XEvent e;
		if (XCheckTypedEvent(D, KeyPress, &e)) {
			L[Q][P]^=4; // remove player from current position
			c=XLookupKeysym(&e.xkey,0); // Lookup key
			if (!(L[Q+=y=(c==KeyD)-(c==KeyU)][P+=x=(c==KeyR)-(c==KeyL)]&1) && L[Q+y][P+x]%8==3) // calculate dx, dy and update player's position, then is next square a box which can be pushed?
				L[Q+y][P+x]^=1,L[Q][P]^=1; // move box
			if (L[Q][P]%8-3) // next square not walkable or occupied?
				P-=x,Q-=y; // reverse player movement
			L[Q][P]^=4; // add player to current/new position
		}
	}
}
