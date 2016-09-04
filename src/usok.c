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

UsokImage imageFromArray(int array[64], UsokColour colours[4]) {
	UsokImage image={.masks[0]=0, .masks[1]=0};

	int x, y, z=0;
	for(y=0;y<8;++y)
		for(x=0;x<8;++x) {
			image.masks[0]|=((array[z]>>0)&1llu)<<z;
			image.masks[1]|=((array[z]>>1)&1llu)<<z;
			++z;
		}

	for(z=0; z<4; ++z)
		image.colours[z]=colours[z];

	return image;
}

UsokImage usokImages[16];

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
	// Create images.
	UsokMask maskTempArray[2];
	UsokColour colourTempArray[4];

	// Create image: wall
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourBlack;
	int arrayWall[64]={
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	usokImages[1]=imageFromArray(arrayWall, colourTempArray);

	// Create image: box
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourDarkGrey;
	colourTempArray[1]=usokColourDarkBlue;
	int arrayBox[64]={
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 1, 1, 1, 1, 0, 0,
		0, 0, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	usokImages[2]=imageFromArray(arrayBox, colourTempArray);

	// Create image: box on goal
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourDarkGrey;
	colourTempArray[1]=usokColourBlack;
	colourTempArray[2]=usokColourDarkBlue;
	int arrayBoxOnGoal[64]={
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 2, 2, 1, 0, 0,
		0, 0, 2, 2, 2, 2, 0, 0,
		0, 0, 2, 2, 2, 2, 0, 0,
		0, 0, 1, 2, 2, 1, 0, 0,
		0, 1, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	usokImages[10]=imageFromArray(arrayBoxOnGoal, colourTempArray);

	// Create image: floor
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourDarkGrey;
	int arrayFloor[64]={
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	usokImages[3]=imageFromArray(arrayFloor, colourTempArray);

	// Create image: goal
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourDarkGrey;
	colourTempArray[1]=usokColourBlack;
	int arrayGoal[64]={
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 1, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 0, 0, 1, 0, 0,
		0, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 1, 0, 0, 1, 0, 0,
		0, 1, 0, 0, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	usokImages[11]=imageFromArray(arrayGoal, colourTempArray);

	// Create image: player
	usokImages[7].masks[0]=0x447C7DFE01297D39llu;
	usokImages[7].masks[1]=0x44006D017D7C0000llu;
	usokImages[7].colours[0]=usokColourDarkGrey;
	usokImages[7].colours[1]=usokColourLightGrey;
	usokImages[7].colours[2]=usokColourLightYellow;
	usokImages[7].colours[3]=usokColourBrown;

	// Create image: player
	// TODO: Fix
	usokImages[15]=usokImages[7];

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




