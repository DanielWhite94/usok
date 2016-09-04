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
	uint32_t colours[4];
} UsokImage;

UsokImage usokImages[]={
        [0]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [1]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x1, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [2]={.m0=0x183c3c180000llu, .m1=0x0llu, .colours[0]=0x5c5c5c, .colours[1]=0xff, .colours[2]=0x0, .colours[3]=0x0},
        [3]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x5c5c5c, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [4]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [5]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [6]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [7]={.m0=0x447c7dfe01297d39llu, .m1=0x44006d017d7c0000llu, .colours[0]=0x5c5c5c, .colours[1]=0x757575, .colours[2]=0xf7e26b, .colours[3]=0x4d2b07},
        [8]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [9]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [10]={.m0=0x42240000244200llu, .m1=0x183c3c180000llu, .colours[0]=0x5c5c5c, .colours[1]=0x1, .colours[2]=0xff, .colours[3]=0x0},
        [11]={.m0=0x42241818244200llu, .m1=0x0llu, .colours[0]=0x5c5c5c, .colours[1]=0x1, .colours[2]=0x0, .colours[3]=0x0},
        [12]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [13]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [14]={.m0=0x0llu, .m1=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [15]={.m0=0x447c7dfe01297d39llu, .m1=0x44006d017d7c0000llu, .colours[0]=0x5c5c5c, .colours[1]=0x757575, .colours[2]=0xf7e26b, .colours[3]=0x4d2b07},
};

Display *disp;
Window window;
GC gc;

#define UsokLevelSize 256
I level[UsokLevelSize][UsokLevelSize]={1};

I playerX, playerY;

void imageDraw(UsokImage image, int x, int y) {
	I i, c;
	for(i=0;i<64;++i)
		if (c=image.colours[((image.m1>>i)%2<<1)|(image.m0>>i)%2])
			XSetForeground(disp, gc, c),
			XFillRectangle(disp, window, gc, x+i%8*USokPixelSize, y+i/8*USokPixelSize, USokPixelSize, USokPixelSize);
}

int main(int argc, char **argv) {
	// Load level
	FILE *file=fopen(argv[1], "r");
	int c, x=0, y=0;
	while((c=fgetc(file))!=EOF) {
		switch(c) {
			case '#': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=1; x++; break;
			case '@': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=7; playerX=x+UsokLevelSize/2, playerY=y+UsokLevelSize/2; x++; break;
			case '+': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=15; playerX=x+UsokLevelSize/2, playerY=y+UsokLevelSize/2; x++; break;
			case '$': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=2; x++; break;
			case '*': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=10; x++; break;
			case '.': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=11; x++; break;
			case ' ': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=3; x++; break;
			case '\n':
				++y;
				x=0;
			break;
		}
	}
	fclose(file);

	// Drawing initialization
	disp=XOpenDisplay(0);
	window=XCreateSimpleWindow(disp,RootWindow(disp,0),0,0, UsokWindowWidth, UsokWindowHeight,0,0,0);
	XSelectInput(disp, window, KeyPressMask);
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

			// Update playerX,Y.
			playerX+=dx;
			playerY+=dy;

			// Next square a box?
			if (!(level[playerY][playerX]&1) && level[playerY+dy][playerX+dx]%8==3) {
				level[playerY+dy][playerX+dx]^=1; // remove from current square
				level[playerY][playerX]^=1; // add to new square
			}

			// Next square not empty and walkable?
			if (level[playerY][playerX]%8!=3) {
				playerX-=dx;
				playerY-=dy;
			}

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




