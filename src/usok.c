#include <stdio.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>

#define I int

#define USokPixelSize 2
#define UsokTileSize (USokPixelSize*8)
#define UsokTilesWide 40
#define UsokTilesHigh 30

I C[]={255,16245355,5057287,7697781,1,6052956};
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
I L[512][512]={1}; // level
I P, Q; // player's X and Y

void imageDraw(long long image[6], I x, I y) {
	I i, c;
	for(i=0;i<64;++i)
		XSetForeground(D, G, C[image[((image[5]>>i)%2<<1)|(image[4]>>i)%2]]),
		XFillRectangle(D, W, G, x+i%8*USokPixelSize, y+i/8*USokPixelSize, USokPixelSize, USokPixelSize);
}

I main(I c, char **v) {
	// load level
	FILE *file=fopen(v[1], "r");
	I x=256, y=256;
	while((c=fgetc(file))!=EOF)
		switch(c) {
			case '#': L[y][x++]=1; break;
			case '@': L[y][x]=7; P=x++, Q=y; break;
			case '+': L[y][x]=15; P=x++, Q=y; break;
			case '$': L[y][x++]=2; break;
			case '*': L[y][x++]=10; break;
			case '.': L[y][x++]=11; break;
			case ' ': L[y][x++]=3; break;
			case '\n':
				++y;
				x=256;
			break;
		}

	// drawing initialization
	D=XOpenDisplay(0);
	XSelectInput(D, W=XCreateSimpleWindow(D,RootWindow(D,0),0,0, UsokTilesWide*UsokTileSize, UsokTilesHigh*UsokTileSize,0,0,0), KeyPressMask);
	XMapWindow(D,W);
	G=XCreateGC(D,W,0,0);

	// main loop
	for(;;) {
		// draw base map
		for(y=-UsokTilesHigh/2; y<=UsokTilesHigh/2+1; ++y)
			for(x=-UsokTilesWide/2; x<=UsokTilesWide/2+1; ++x)
				imageDraw(IM[L[y+256][x+256]], UsokTileSize*(x+UsokTilesWide/2), UsokTileSize*(y+UsokTilesHigh/2));

		// wait for key press
		XEvent e;
		XNextEvent(D, &e);
		if(e.type==KeyPress) {
			L[Q][P]^=4; // remove player from current position
			c=XLookupKeysym(&e.xkey,0); // Lookup key
			P+=x=(c==KeyR)-(c==KeyL); // calculate dx, dy and update player's position.
			Q+=y=(c==KeyD)-(c==KeyU);
			if (!(L[Q][P]&1) && L[Q+y][P+x]%8==3) // next square a box which can be pushed?
				L[Q+y][P+x]^=1,L[Q][P]^=1; // move box
			if (L[Q][P]%8-3) // next square not walkable or occupied?
				P-=x,Q-=y; // reverse player movement
			L[Q][P]^=4; // add player to current/new position
		}
	}
}
