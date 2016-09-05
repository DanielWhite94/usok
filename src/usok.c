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
		if (c=C[image[((image[5]>>i)%2<<1)|(image[4]>>i)%2]])
			XSetForeground(D, G, c),
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
		I dx, dy;
		for(dy=-UsokTilesHigh/2; dy<=UsokTilesHigh/2+1; ++dy)
			for(dx=-UsokTilesWide/2; dx<=UsokTilesWide/2+1; ++dx)
				imageDraw(IM[L[dy+256][dx+256]], UsokTileSize*(dx+UsokTilesWide/2), UsokTileSize*(dy+UsokTilesHigh/2));

		// wait for key press
		XEvent event;
		XNextEvent(D, &event);
		if(event.type==KeyPress) {
			L[Q][P]^=4; // remove player from current position
			I key=XLookupKeysym(&event.xkey,0); // Lookup key
			P+=dx=(key==KeyR)-(key==KeyL); // calculate dx, dy and update player's position.
			Q+=dy=(key==KeyD)-(key==KeyU);
			if (!(L[Q][P]&1) && L[Q+dy][P+dx]%8==3) // next square a box which can be pushed?
				L[Q+dy][P+dx]^=1,L[Q][P]^=1; // move box
			if (L[Q][P]%8-3) // next square not walkable or occupied?
				P-=dx,Q-=dy; // reverse player movement
			L[Q][P]^=4; // add player to current/new position
		}
	}
}
