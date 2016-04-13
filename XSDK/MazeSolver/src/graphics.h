#include "xemaclite.h"
#include "main.h"
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

void init_graphics();
void print_world(u64* maze);
void drawWall(int x, int y, int dir, int length, int grid_height, int grid_width);
void drawWaypoint(int x, int y);
void drawStart(int x, int y);
void drawGrid(int height, int width);
void draw(world_t* world);

#endif
