

typedef struct {
	u8 id;
	u8 size;
	u8 width;
	u8 height;
	u8 num_waypoints;
	u8 waypoints[12][2];
	u8 num_walls;
	u8 walls[20][4];
	u8 start_x;
	u8 start_y;
} world_t;
