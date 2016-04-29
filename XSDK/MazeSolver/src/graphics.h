#include "xemaclite.h"
#include "main.h"
#ifndef GRAPHICS_H_
#define GRAPHICS_H_

void init_graphics();
void print_world(u64* maze);
void draw_wall(int x, int y, int dir, int length, int grid_height, int grid_width);
void draw_waypoint(int x, int y, u8 color);
void drawStart(int x, int y);
void drawGrid(int height, int width);
void draw(world_t* world);
void draw_border(int height, int width, u8 colour);
void draw_path(int x, int y);
void draw_border_solution(world_t* world, int mark);
#endif
