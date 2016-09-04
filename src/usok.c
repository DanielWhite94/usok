#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define I int

#define UsokZoom 2 // integer in range [1,8]
#define USokPixelSize UsokZoom
#define UsokImageSize 8
#define UsokTileSize (USokPixelSize*UsokImageSize)
#define UsokTilesWide 40
#define UsokTilesHigh 30
#define UsokWindowWidth (UsokTilesWide*UsokTileSize)
#define UsokWindowHeight (UsokTilesHigh*UsokTileSize)

typedef struct {
	uint64_t m0, m1;
	uint32_t c[4];
} UsokImage;

UsokImage usokImages[]={
        [1]={.c[0]=0x1},
        [2]={.m0=0x183c3c180000llu, .c[0]=0x5c5c5c, .c[1]=0xff},
        [3]={.c[0]=0x5c5c5c},
        [7]={.m0=0x447c7dfe01297d39llu, .m1=0x44006d017d7c0000llu, .c[0]=0x5c5c5c, .c[1]=0x757575, .c[2]=0xf7e26b, .c[3]=0x4d2b07},
        [10]={.m0=0x42240000244200llu, .m1=0x183c3c180000llu, .c[0]=0x5c5c5c, .c[1]=0x1, .c[2]=0xff},
        [11]={.m0=0x42241818244200llu, .c[0]=0x5c5c5c, .c[1]=0x1},
        [15]={.m0=0x447c7dfe01297d39llu, .m1=0x44006d017d7c0000llu, .c[0]=0x5c5c5c, .c[1]=0x757575, .c[2]=0xf7e26b, .c[3]=0x4d2b07},
};

Display *disp;
Window window;
GC gc;

I level[512][512]={1};

I playerX, playerY;

void imageDraw(UsokImage image, int x, int y) {
	I i, c;
	for(i=0;i<64;++i)
		if (c=image.c[((image.m1>>i)%2<<1)|(image.m0>>i)%2])
			XSetForeground(disp, gc, c),
			XFillRectangle(disp, window, gc, x+i%8*USokPixelSize, y+i/8*USokPixelSize, USokPixelSize, USokPixelSize);
}

int main(int argc, char **argv) {
	// Load level
	FILE *file=fopen(argv[1], "r");
	int c, x=0, y=0;
	while((c=fgetc(file))!=EOF) {
		switch(c) {
			case '#': level[y+256][x+256]=1; x++; break;
			case '@': level[y+256][x+256]=7; playerX=x+256, playerY=y+256; x++; break;
			case '+': level[y+256][x+256]=15; playerX=x+256, playerY=y+256; x++; break;
			case '$': level[y+256][x+256]=2; x++; break;
			case '*': level[y+256][x+256]=10; x++; break;
			case '.': level[y+256][x+256]=11; x++; break;
			case ' ': level[y+256][x+256]=3; x++; break;
			case '\n':
				++y;
				x=0;
			break;
		}
	}
	fclose(file);

	// Drawing initialization
	disp=XOpenDisplay(0);
	XSelectInput(disp, window=XCreateSimpleWindow(disp,RootWindow(disp,0),0,0, UsokWindowWidth, UsokWindowHeight,0,0,0), KeyPressMask);
	XMapWindow(disp,window);
	gc=XCreateGC(disp,window,0,0);

	// Main loop
	XEvent event;
	for(;;XNextEvent(disp, &event)) {
		if(event.type==KeyPress) {
			// Lookup key
			int key=XLookupKeysym(&event.xkey,0);
			int dx=(key==KeyR)-(key==KeyL);
			int dy=(key==KeyD)-(key==KeyU);

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
		}

		// Draw base map
		I dx, dy;
		for(dy=-UsokTilesHigh/2; dy<=UsokTilesHigh/2+1; ++dy)
			for(dx=-UsokTilesWide/2; dx<=UsokTilesWide/2+1; ++dx) {
				int tx=dx+UsokLevelSize/2;
				int ty=dy+UsokLevelSize/2;
				int sx=UsokTileSize*(dx+UsokTilesWide/2);
				int sy=UsokTileSize*(dy+UsokTilesHigh/2);
				imageDraw(usokImages[level[ty][tx]], sx, sy);
			}

		// Update disp
		XFlush(disp);

		// Delay
		usleep(1000*1000.0/8);
	}
}




