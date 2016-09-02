#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <unistd.h>

#define I int
#define D double

#define UsokZoom 2 // integer in range [1,8]
#define USokPixelSize UsokZoom
#define UsokImageSize 8
#define UsokTileSize (USokPixelSize*UsokImageSize)
#define UsokTilesWide 40
#define UsokTilesHigh 30
#define UsokWindowWidth (UsokTilesWide*UsokTileSize)
#define UsokWindowHeight (UsokTilesHigh*UsokTileSize)

typedef enum {
	TileTypeBasePlayer=1,
	TileTypeBaseWalkable=2,
	TileTypeBaseGoal=4,
	TileTypeBaseNoBox=8,

	TileTypeBox=TileTypeBaseWalkable,
	TileTypeBoxOnGoal=TileTypeBaseWalkable|TileTypeBaseGoal,
	TileTypeWall=TileTypeBaseNoBox,
	TileTypeFloor=TileTypeBaseNoBox|TileTypeBaseWalkable,
	TileTypePlayer=TileTypeBaseNoBox|TileTypeBaseWalkable|TileTypeBasePlayer,
	TileTypeGoal=TileTypeBaseNoBox|TileTypeBaseGoal|TileTypeBaseWalkable,
	TileTypePlayerOnGoal=TileTypeBaseNoBox|TileTypeBaseGoal|TileTypeBaseWalkable|TileTypeBasePlayer,

	TileTypeNB=16,
} TileType;

#define TILETYPEEMPTY(t) (((t)&(TileTypeBaseNoBox|TileTypeBaseWalkable))==(TileTypeBaseNoBox|TileTypeBaseWalkable))

typedef union {
	uint8_t components[4];
	uint32_t combined;
} UsokColour;

UsokColour usokColourLightBlue={.combined=0x8888FF};
UsokColour usokColourDarkBlue={.combined=0x0000FF};
UsokColour usokColourLightGreen={.combined=0x44FF44};
UsokColour usokColourDarkGreen={.combined=0x00DD00};
UsokColour usokColourLightYellow={.combined=0xf7e26b};
UsokColour usokColourDarkYellow={.combined=0xFFFF00};
UsokColour usokColourBrown={.combined=0x4d2b07};
UsokColour usokColourLightGrey={.combined=0x757575};
UsokColour usokColourBlack={.combined=0x000001};
UsokColour usokColourTransparent={.combined=0x000000};
UsokColour usokColourDarkDarkGreen={.combined=0x184a1d};
UsokColour usokColourDarkGrey={.combined=0x5c5c5c};
UsokColour usokColourPink={.combined=0xff26db};
UsokColour usokColourOrange={.combined=0xeb8931};

typedef uint64_t UsokMask;

typedef struct {
	UsokMask masks[3];
	UsokColour colours[8];
} UsokImage;

UsokImage imageFromMasks(UsokMask masks[3], UsokColour colours[8]) {
	UsokImage image;
	I i;
	for(i=0;i<3;++i)
		image.masks[i]=masks[i];
	for(i=0;i<8;++i)
		image.colours[i]=colours[i];

	return image;
}

UsokImage imageFromArray(int array[64], UsokColour colours[8]) {
	UsokMask masks[3]={0,0,0};

	int x, y, z=0;
	for(y=0;y<8;++y)
		for(x=0;x<8;++x) {
			masks[0]|=((array[z]>>0)&1llu)<<z;
			masks[1]|=((array[z]>>1)&1llu)<<z;
			masks[2]|=((array[z]>>2)&1llu)<<z;
			++z;
		}

	return imageFromMasks(masks, colours);
}

UsokImage usokImages[TileTypeNB];

Display *disp;
Window window;
GC gc;

#define UsokLevelSize 256
TileType level[UsokLevelSize][UsokLevelSize];

I playerX, playerY;

void imageDraw(UsokImage image, int x, int y) {
	I i;
	UsokColour c;
	for(i=0;i<64;++i)
		if ((c=image.colours[
				((image.masks[2]>>i)%2<<2)|
				((image.masks[1]>>i)%2<<1)|
				 (image.masks[0]>>i)%2     ]).combined!=0)
			XSetForeground(disp, gc, c.combined),
			XFillRectangle(disp, window, gc, x+(i%8)*USokPixelSize, y+(i/8)*USokPixelSize, USokPixelSize, USokPixelSize);
}

int main(int argc, char **argv) {
	// Create images.
	UsokMask maskTempArray[3];
	UsokColour colourTempArray[8];

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
	usokImages[TileTypeWall]=imageFromArray(arrayWall, colourTempArray);

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
	usokImages[TileTypeBox]=imageFromArray(arrayBox, colourTempArray);

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
	usokImages[TileTypeBoxOnGoal]=imageFromArray(arrayBoxOnGoal, colourTempArray);

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
	usokImages[TileTypeFloor]=imageFromArray(arrayFloor, colourTempArray);

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
	usokImages[TileTypeGoal]=imageFromArray(arrayGoal, colourTempArray);

	// Create image: player
	memset(maskTempArray, 0, sizeof(maskTempArray));
	maskTempArray[0]=0x447C7DFE01297D39llu;
	maskTempArray[1]=0x44006D017D7C0000llu;
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourDarkGrey;
	colourTempArray[1]=usokColourLightGrey;
	colourTempArray[2]=usokColourLightYellow;
	colourTempArray[3]=usokColourBrown;
	usokImages[TileTypePlayer]=imageFromMasks(maskTempArray, colourTempArray);

	// Create image: player
	// TODO: Fix
	memset(maskTempArray, 0, sizeof(maskTempArray));
	maskTempArray[0]=0x447C7DFE01297D39llu;
	maskTempArray[1]=0x44006D017D7C0000llu;
	memset(colourTempArray, 0, sizeof(colourTempArray));
	colourTempArray[0]=usokColourDarkGrey;
	colourTempArray[1]=usokColourLightGrey;
	colourTempArray[2]=usokColourLightYellow;
	colourTempArray[3]=usokColourBrown;
	usokImages[TileTypePlayerOnGoal]=imageFromMasks(maskTempArray, colourTempArray);

	// Create level
	I x,y;
	for(y=0;y<UsokLevelSize;++y)
		for(x=0;x<UsokLevelSize;++x)
			level[x][y]=TileTypeWall;

	FILE *file=fopen(argv[1], "r");
	int c;
	x=0, y=0;
	while((c=fgetc(file))!=EOF) {
		switch(c) {
			case '#': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypeWall; x++; break;
			case '@': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypePlayer; playerX=x+UsokLevelSize/2, playerY=y+UsokLevelSize/2; x++; break;
			case '+': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypePlayerOnGoal; playerX=x+UsokLevelSize/2, playerY=y+UsokLevelSize/2; x++; break;
			case '$': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypeBox; x++; break;
			case '*': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypeBoxOnGoal; x++; break;
			case '.': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypeGoal; x++; break;
			case ' ': level[y+UsokLevelSize/2][x+UsokLevelSize/2]=TileTypeFloor; x++; break;
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
			level[playerY][playerX]&=~TileTypeBasePlayer;

			// Update playerX,Y.
			playerX+=dx;
			playerY+=dy;

			// Next square a box?
			if (!(level[playerY][playerX]&TileTypeBaseNoBox)) {
				if (TILETYPEEMPTY(level[playerY+dy][playerX+dx])) {
					// can be moved, move
					level[playerY+dy][playerX+dx]&=~TileTypeBaseNoBox;
					level[playerY][playerX]|=TileTypeBaseNoBox;
				} else {
					// we abort player moving below (as square not empty)
				}
			}

			// Next square not empty and walkable?
			if (!TILETYPEEMPTY(level[playerY][playerX])) {
				playerX-=dx;
				playerY-=dy;
			}

			// Add player to (potentially new) x,y.
			level[playerY][playerX]|=TileTypeBasePlayer;
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




