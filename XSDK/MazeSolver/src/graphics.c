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
void draw_background();
void draw_border(int height, int width, u8 colour);
void draw_rect(int xLoc, int yLoc, int width, int height, u8 colour);
void draw_dot(int x, int y, u8 colour);

/*
 * Method to initialise graphic address and buffer
 */
void init_graphics(){
    *((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR + 1) = 1;
    *((volatile unsigned int *) XPAR_EMBS_VGA_0_BASEADDR) = FRAME_BUFFER;
}

/*
 * Method to draw world, including grid, borders, waypoints, starting_waypoint and walls
 */
void draw(world_t* world){
	// Scales to fill most of screen
	CELL_DIM = (HEIGHT-1)/world->width;
	drawGrid(world->width, world->height);

	int i;
	for (i=0; i< world->num_waypoints; i++){
		if (i==0){
			draw_waypoint(world->waypoints[i].x, world->waypoints[i].y, GREEN);
		} else {
			draw_waypoint(world->waypoints[i].x, world->waypoints[i].y, MAGENTA);
		}
	}

	for (i=0; i< world->num_walls; i++){
		draw_wall(world->walls[i].coords.x, world->walls[i].coords.y, world->walls[i].dir, world->walls[i].len, world->width, world->height);
	}
}

/*
 * Method to draw dotted grid of dimensions corresponding to world size
 */
void drawGrid(int width, int height){
	draw_background();
	int x, y;
	for (x=0; x<width+1; x++){
		for (y=0; y<height+1; y++){
			draw_dot(x*CELL_DIM, y*CELL_DIM, BLACK);
		}
	}
	draw_border(height, width, BLACK);
}

/*
 * Method to draw a wall in the world
 */
void draw_wall(int x, int y, int dir, int length, int grid_width, int grid_height){
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
		draw_rect(x+1, y+1, CELL_DIM-1, CELL_DIM-1, BLACK);
		if (dir==0){
			x+=CELL_DIM;
		} else {
			y+=CELL_DIM;
		}

	}
}

/*
 * Method to draw white background
 */
void draw_background(){
	draw_rect(0, 0, WIDTH, HEIGHT, WHITE);
}

/*
 * Change border colour based on response from server
 */
void draw_border_solution(world_t* world, int mark){
	draw_border(world->height, world->width, mark==0 ? GREEN : RED);
}


/*
 * Method to draw border around dotted grid
 */
void draw_border(int height, int width, u8 colour){
	int x, y;
	x = y = 0;
	for (y = -1; y<(CELL_DIM * height) + 1; y++){
		draw_dot(-1, y, colour);
		draw_dot((CELL_DIM * width)+1, y, colour);
	}
	x = y = 0;
	for (x = -1; x<(CELL_DIM * width) + 1; x++){
		draw_dot(x, -1, colour);
		draw_dot(x, (CELL_DIM * height) + 1, colour);
	}
}

/*
 * Method to draw waypoint in grid
 */
void draw_waypoint(int x, int y, u8 color){
	draw_rect((x*CELL_DIM) + 1, (y*CELL_DIM) + 1, CELL_DIM-1, CELL_DIM-1, color);
}


/*
 * Method to draw a rectangle
 */
void draw_rect(int xLoc, int yLoc, int width, int height, u8 colour) {
	int x, y;

    for (y = yLoc; y < yLoc + height; y++) {
        for (x = xLoc; x < xLoc + width; x++) {
            draw_dot(x, y, colour);
        }
    }
}

/*
 * Method to draw a dotted red path in world
 */
void draw_path(int x, int y){
	x = (x * CELL_DIM) + (CELL_DIM/2);
	y = (y * CELL_DIM) + (CELL_DIM/2);
	int i, i2;
	for (i = x-1; i<=x+2; i++){
		for (i2 = y-1; i2<=y+2; i2++){
			draw_dot(i, i2, RED);
		}
	}
}

/*
 * Method to draw an individual dot of colour
 */
void draw_dot(int x, int y, u8 colour) {
	*((volatile u8 *) FRAME_BUFFER + x + (WIDTH * (y))) = colour;
}
