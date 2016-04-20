
#ifndef MAIN_H_
#define MAIN_H_

typedef struct __attribute__((packed)){
	u8 x;
	u8 y;
} point_t;

typedef struct __attribute__((packed)){
	point_t coords;
	u8 dir;
	u8 len;
} wall_t;

typedef struct __attribute__((packed)){
	u32 id;
	u8 size;
	u8 width;
	u8 height;
	u8 num_waypoints;
	point_t waypoints[12];
	u8 num_walls;
	wall_t walls[20];
} world_t;




#endif /* MAIN_H_ */
