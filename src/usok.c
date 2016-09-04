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

#define TILETYPEEMPTY(t) (((t)&3)==3)

typedef uint32_t UsokColour;

const UsokColour usokColourLightBlue=0x8888FF;
const UsokColour usokColourDarkBlue=0x0000FF;
const UsokColour usokColourLightGreen=0x44FF44;
const UsokColour usokColourDarkGreen=0x00DD00;
const UsokColour usokColourLightYellow=0xf7e26b;
const UsokColour usokColourDarkYellow=0xFFFF00;
const UsokColour usokColourBrown=0x4d2b07;
const UsokColour usokColourLightGrey=0x757575;
const UsokColour usokColourBlack=0x000001;
const UsokColour usokColourTransparent=0x000000;
const UsokColour usokColourDarkDarkGreen=0x184a1d;
const UsokColour usokColourDarkGrey=0x5c5c5c;
const UsokColour usokColourPink=0xff26db;
const UsokColour usokColourOrange=0xeb8931;

typedef uint64_t UsokMask;

typedef struct {
	UsokMask masks[2];
	UsokColour colours[4];
} UsokImage;

UsokImage usokImages[]={
        [0]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [1]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x1, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [2]={.masks[0]=0x183c3c180000llu, .masks[1]=0x0llu, .colours[0]=0x5c5c5c, .colours[1]=0xff, .colours[2]=0x0, .colours[3]=0x0},
        [3]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x5c5c5c, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [4]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [5]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [6]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [7]={.masks[0]=0x447c7dfe01297d39llu, .masks[1]=0x44006d017d7c0000llu, .colours[0]=0x5c5c5c, .colours[1]=0x757575, .colours[2]=0xf7e26b, .colours[3]=0x4d2b07},
        [8]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [9]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [10]={.masks[0]=0x42240000244200llu, .masks[1]=0x183c3c180000llu, .colours[0]=0x5c5c5c, .colours[1]=0x1, .colours[2]=0xff, .colours[3]=0x0},
        [11]={.masks[0]=0x42241818244200llu, .masks[1]=0x0llu, .colours[0]=0x5c5c5c, .colours[1]=0x1, .colours[2]=0x0, .colours[3]=0x0},
        [12]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [13]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [14]={.masks[0]=0x0llu, .masks[1]=0x0llu, .colours[0]=0x0, .colours[1]=0x0, .colours[2]=0x0, .colours[3]=0x0},
        [15]={.masks[0]=0x447c7dfe01297d39llu, .masks[1]=0x44006d017d7c0000llu, .colours[0]=0x5c5c5c, .colours[1]=0x757575, .colours[2]=0xf7e26b, .colours[3]=0x4d2b07},
};

Display *disp;
Window window;
GC gc;

#define UsokLevelSize 256
I level[UsokLevelSize][UsokLevelSize]={1};

I playerX, playerY;

void imageDraw(UsokImage image, int x, int y) {
	I i;
	UsokColour c;
	for(i=0;i<64;++i)
		if ((c=image.colours[((image.masks[1]>>i)%2<<1)|(image.masks[0]>>i)%2])!=0)
			XSetForeground(disp, gc, c),
			XFillRectangle(disp, window, gc, x+(i%8)*USokPixelSize, y+(i/8)*USokPixelSize, USokPixelSize, USokPixelSize);
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
			level[playerY][playerX]&=~4;

			// Update playerX,Y.
			playerX+=dx;
			playerY+=dy;

			// Next square a box?
			if (!(level[playerY][playerX]&1) && TILETYPEEMPTY(level[playerY+dy][playerX+dx])) {
				level[playerY+dy][playerX+dx]^=1; // remove from current square
				level[playerY][playerX]^=1; // add to new square
			}

			// Next square not empty and walkable?
			if (!TILETYPEEMPTY(level[playerY][playerX])) {
				playerX-=dx;
				playerY-=dy;
			}

			// Add player to (potentially new) x,y.
			level[playerY][playerX]|=4;
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




