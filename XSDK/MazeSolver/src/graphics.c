#include "platform.h"
#include "xuartlite_l.h"
#include "graphics.h"
#include "main.h"
///////////////////////////////////////////////////
// Set these values to match those chosen in XPS //
///////////////////////////////////////////////////
#define WIDTH          800
#define HEIGHT         600
#define BITS_PER_PIXEL 8
///////////////////////////////////////////////////

int CELL_DIM = HEIGHT/60;

// Set frame buffer location to start of DDR
#define FRAME_BUFFER XPAR_DDR_SDRAM_MPMC_BASEADDR

// Colour definitions (2 pixels per byte)
#define BLACK   0b00000000
#define RED     0b00000100
#define WHITE   0b01110111
#define GREEN   0b00000010
#define BLUE    0b00010001
#define CYAN    0b00110011
#define YELLOW  0b01100110
#define MAGENTA 0b01010101

// Signatures
void drawBackground();
void drawBorder(int height, int width, u8 colour);
void drawRect(int xLoc, int yLoc, int width, int height, u8 colour);
void drawDot(int x, int y, u8 colour);

void init_graphics(){
    *((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR + 1) = 1;
    *((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR) = FRAME_BUFFER;
}

void draw(world_t* world){
	xil_printf("Drawing\n\r");
	CELL_DIM = (HEIGHT-1)/world->width;
	drawGrid(world->width, world->height);

	int i;

	for (i=0; i< world->num_waypoints; i++){
		if (i==0){
			drawWaypoint(world->waypoints[i].x, world->waypoints[i].y, GREEN);
		} else {
			drawWaypoint(world->waypoints[i].x, world->waypoints[i].y, MAGENTA);
		}
	}

	for (i=0; i< world->num_walls; i++){
		drawWall(world->walls[i].x, world->walls[i].y, world->walls[i].dir, world->walls[i].len, world->width, world->height);
	}
}


void drawGrid(int width, int height){
	drawBackground();
	int x, y;
	for (x=0; x<width+1; x++){
		for (y=0; y<height+1; y++){
			drawDot(x*CELL_DIM, y*CELL_DIM, BLACK);
		}
	}
	drawBorder(height, width, BLACK);
}

void drawWall(int x, int y, int dir, int length, int grid_width, int grid_height){
	if (dir==0){
		if (x+length>grid_width){
			length = grid_width-x;
		}
	} else {
		if (y+length>grid_height){
			length = grid_height-y;
		}
	}

	x *= CELL_DIM;
	y *= CELL_DIM;
	int i;
	for (i=0; i<length; i++){
		drawRect(x+1, y+1, CELL_DIM-1, CELL_DIM-1, BLACK);
		if (dir==0){
			x+=CELL_DIM;
		} else {
			y+=CELL_DIM;
		}

	}
}

void drawBackground(){
	drawRect(0, 0, WIDTH, HEIGHT, WHITE);
}

void drawBorder(int height, int width, u8 colour){
	int x, y;
	x = y = 0;
	for (y = -1; y<(CELL_DIM * height) + 1; y++){
		drawDot(-1, y, colour);
		drawDot((CELL_DIM * width)+1, y, colour);
	}
	x = y = 0;
	for (x = -1; x<(CELL_DIM * width) + 1; x++){
		drawDot(x, -1, colour);
		drawDot(x, (CELL_DIM * height) + 1, colour);
	}
}

void drawWaypoint(int x, int y, u8 color){
	drawRect((x*CELL_DIM) + 1, (y*CELL_DIM) + 1, CELL_DIM-1, CELL_DIM-1, color);
}


// Draws a rectangle of solid colour on the screen
void drawRect(int xLoc, int yLoc, int width, int height, u8 colour) {
	int x, y;

    for (y = yLoc; y < yLoc + height; y++) {
        for (x = xLoc; x < xLoc + width; x++) {
            drawDot(x, y, colour);
        }
    }
}

void drawPath(int x, int y){
	x = (x * CELL_DIM) + (CELL_DIM/2);
	y = (y * CELL_DIM) + (CELL_DIM/2);
	int i, i2;
	for (i = x-2; i<=x+2; i++){
		for (i2 = y-2; i2<=y+2; i2++){
			drawDot(i, i2, RED);
//			*((volatile u8 *) FRAME_BUFFER + CORNER_MARGIN + i + (WIDTH * (i2+CORNER_MARGIN))) = RED;
		}
	}
}

void drawDot(int x, int y, u8 colour) {
	*((volatile u8 *) FRAME_BUFFER + x + (WIDTH * (y))) = colour;
}
