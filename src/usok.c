#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define I int

#define USokPixelSize 2
#define UsokTileSize (USokPixelSize*8)
#define UsokTilesWide 40
#define UsokTilesHigh 30

typedef struct {
	uint64_t m0, m1;
	uint32_t c[4];
} UsokImage;

UsokImage usokImages[]={
        [1]={.c[0]=1},
        [2]={.m0=0x183c3c180000ll, .c[0]=6052956, .c[1]=255},
        [3]={.c[0]=6052956},
        [7]={.m0=0x447c7dfe01297d39ll, .m1=0x44006d017d7c0000ll, .c[0]=6052956, .c[1]=7697781, .c[2]=16245355, .c[3]=5057287},
        [10]={.m0=0x42240000244200ll, .m1=0x183c3c180000ll, .c[0]=6052956, .c[1]=1, .c[2]=255},
        [11]={.m0=0x42241818244200ll, .c[0]=6052956, .c[1]=1},
        [15]={.m0=0x447c7dfe01297d39ll, .m1=0x44006d017d7c0000ll, .c[0]=6052956, .c[1]=7697781, .c[2]=16245355, .c[3]=5057287},
};

Display *disp;
Window window;
GC gc;

I level[512][512]={1};

I playerX, playerY;

void imageDraw(UsokImage image, I x, I y) {
	I i, c;
	for(i=0;i<64;++i)
		if (c=image.c[((image.m1>>i)%2<<1)|(image.m0>>i)%2])
			XSetForeground(disp, gc, c),
			XFillRectangle(disp, window, gc, x+i%8*USokPixelSize, y+i/8*USokPixelSize, USokPixelSize, USokPixelSize);
}

I main(I argc, char **argv) {
	// Load level
	FILE *file=fopen(argv[1], "r");
	I c, x=256, y=256;
	while((c=fgetc(file))!=EOF)
		switch(c) {
			case '#': level[y][x++]=1; break;
			case '@': level[y][x]=7; playerX=x++, playerY=y; break;
			case '+': level[y][x]=15; playerX=x++, playerY=y; break;
			case '$': level[y][x++]=2; break;
			case '*': level[y][x++]=10; break;
			case '.': level[y][x++]=11; break;
			case ' ': level[y][x++]=3; break;
			case '\n':
				++y;
				x=256;
			break;
		}

	// Drawing initialization
	disp=XOpenDisplay(0);
	XSelectInput(disp, window=XCreateSimpleWindow(disp,RootWindow(disp,0),0,0, UsokTilesWide*UsokTileSize, UsokTilesHigh*UsokTileSize,0,0,0), KeyPressMask);
	XMapWindow(disp,window);
	gc=XCreateGC(disp,window,0,0);

	// Main loop
	for(;;) {
		// Draw base map
		I dx, dy;
		for(dy=-UsokTilesHigh/2; dy<=UsokTilesHigh/2+1; ++dy)
			for(dx=-UsokTilesWide/2; dx<=UsokTilesWide/2+1; ++dx)
				imageDraw(usokImages[level[dy+256][dx+256]], UsokTileSize*(dx+UsokTilesWide/2), UsokTileSize*(dy+UsokTilesHigh/2));

		// Update disp
		XFlush(disp);

		// Look for key presses.
		XEvent event;
		XNextEvent(disp, &event);
		if(event.type!=KeyPress)
			continue;

		// Remove player from current x,y.
		level[playerY][playerX]^=4;

		// Lookup key and update playerX,Y.
		I key=XLookupKeysym(&event.xkey,0);
		playerX+=dx=(key==KeyR)-(key==KeyL);
		playerY+=dy=(key==KeyD)-(key==KeyU);

		// Next square a box?
		if (!(level[playerY][playerX]&1) && level[playerY+dy][playerX+dx]%8==3)
			level[playerY+dy][playerX+dx]^=1, // remove from current square
			level[playerY][playerX]^=1; // add to new square

		// Next square not empty and walkable?
		if (level[playerY][playerX]%8!=3)
			playerX-=dx,
			playerY-=dy;

		// Add player to (potentially new) x,y.
		level[playerY][playerX]^=4;

		// Delay
		usleep(99999);
	}
}



