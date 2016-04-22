#include "toplevel.h"

//Prototypes
void close_node(int index, int* num_open, node_t* nodes);
int get_best_open_node(point_t target, node_t* nodes);
int manhattan(u8 x, u8 y, point_t* point2);
void check_neighbours(u12 index, int* num_open, node_t* nodes, point_t* target,
		u16 cost);
u1 is_openable(u8 x, u8 y, node_t* nodes);
u1 is_wall(u8 x, u8 y);
void open_node(u8 x, u8 y, u12 cost, int* num_open, node_t* nodes,
		point_t* target, u12 parent_index);
void affect_neighbour(u8 x, u8 y, u12 cost, int* num_open, node_t* nodes,
		point_t* target, u12 parent_index);
u16 get_shortest_path(point_t w1, point_t w2, u1 get_path,
		hls::stream<uint32> &output);

wall_t walls[20];
uint32 num_walls;
uint32 read_walls;

point_t waypoints[12];
u4 num_waypoints;
uint32 read_waypoints;

u16 distance_matrix[12][12];

u4 best_tour[12];
u8 grid_size;

int NUM_NODES = 3600;
node_t nodes[3600];

//Top-level function
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output) {
#pragma HLS RESOURCE variable=input core=AXI4Stream
#pragma HLS RESOURCE variable=output core=AXI4Stream
#pragma HLS INTERFACE ap_ctrl_none port=return
	int num_read = 0;
	// Initialise all variables
	int i, i2;
	for (i=0; i<12; i++){
		best_tour[i] = i;
		for (i2=0; i2<12; i2++){
			distance_matrix[i][i2]=0;
		}
	}
	for (i=0; i<NUM_NODES; i++){
		nodes[i].set=0;
	}


	//Read in NUMDATA items
	uint32 in = input.read();
	// Decompose into 8 bit values
	grid_size = (in >> 16) & 0xFF;
	num_waypoints = (in >> 8) & 0xFF;
	num_walls = in & 0xFF;

	// Read in all waypoints/walls
	in = input.read();
	for (i=0; i< num_walls+(num_waypoints/2); i++){
		if (read_waypoints < num_waypoints) {
			waypoints[read_waypoints].x = in & 0xFF;
			waypoints[read_waypoints].y = (in >> 8) & 0xFF;
			if (read_waypoints < num_waypoints) {
				waypoints[read_waypoints].x = (in >> 16) & 0xFF;
				waypoints[read_waypoints].y = (in >> 24) & 0xFF;
			}
		} else if (read_walls < num_walls) {
			walls[read_walls].x = in & 0xFF;
			walls[read_walls].y = (in >> 8) & 0xFF;
			walls[read_walls].dir = (in >> 16) & 0xFF;
			walls[read_walls].len = (in >> 24) & 0xFF;
			read_walls++;
		}
		if (read_walls == num_walls) {
			break;
		}
		in = input.read();
	}

// Set active, 0 cost and starting waypoint
	for (u8 w1 = 0; w1 < num_waypoints; w1++) {
		for (u8 w2 = w1; w2 < num_waypoints; w2++) {
			if (w1 != w2) {
				// Perform A* Between every pair of waypoints and save symetrically in distance matrix
				distance_matrix[w1][w2] = get_shortest_path(waypoints[w1],
						waypoints[w2], 0, output);
				distance_matrix[w2][w1] = distance_matrix[w1][w2];
			}
		}
	}

	// Calculate shortest hamiltonian cycle
	uint32 shortest_loop = get_shortest_loop(distance_matrix, num_waypoints,
			best_tour);
	printf("Best solution length: %d \n\r", (int) shortest_loop);

	// Output solution to microblaze for verification with server
	output.write(shortest_loop);

	// Then perform A* between pairs of waypoints to retrieve path
	for (i = 0; i < num_waypoints; i++) {
		if (i != num_waypoints - 1) {
			get_shortest_path(waypoints[best_tour[i]],
					waypoints[best_tour[i + 1]], 1, output);
		} else {
			get_shortest_path(waypoints[best_tour[i]], waypoints[best_tour[0]],
					1, output);
		}
	}
	// Write end of communication packet
	output.write((uint32) 0xFFFF);
	return;
}

// Convert an index to X coordinate
u8 i_to_x(u12 index){
	u12 i = index;
	while (i>=60){
		i-=60;
	}
	return i;
}

// Convert index to Y coordinate
u8 i_to_y(u12 index){
	index = (index) - index%60;
	u12 i = 0;
	while (index>=60){
		index-=60;
		i++;
	}
	return i;
}

// Convert coordinates to index
u12 xy_to_i(u8 x, u8 y){
	return (y*60) + x;
}

// A* between w1 and w2, optionally can return path through AXI stream
u16 get_shortest_path(point_t w1, point_t w2, u1 get_path,
		hls::stream<uint32> &output) {

	for (int n = 0; n < NUM_NODES; n++) {
		nodes[n].set = 0;
	}
	u12 min_len = 0;

	node_t initial;
	initial.set = 2;
	initial.cost = 0;
	u12 index = xy_to_i(w1.x, w1.y);
	initial.prev = index;

	nodes[index] = initial;
	int num_open = 1;

	while (num_open > 0) {
		u8 x;
		u8 y;

		index = get_best_open_node(w2, nodes);
		x = i_to_x(index);
		y = i_to_y(index);
		if ((x == w2.x) & (y == w2.y)) {
			min_len = nodes[index].cost;
			break;
		} else {
			check_neighbours(index, &num_open, nodes, &w2, nodes[index].cost);
		}
		close_node(index, &num_open, nodes);
	}
	if (get_path) {
		while (1) {
			uint32 out = 0;
			out = (0xFFFF & ((uint32) i_to_x(index))) << 16;
			out |= (0xFFFF & (((uint32) i_to_y(index))));
			output.write(out);
			if (index == nodes[index].prev) {
				break;
			} else {
				index = nodes[index].prev;
			}

		}
	}
	return min_len;
}

// Expand node and potentially open/update neighbours
void check_neighbours(u12 index, int* num_open, node_t* nodes, point_t* target,
		u16 cost) {
	u8 x = i_to_x(index);
	u8 y = i_to_y(index);
	affect_neighbour(x + 1, y, cost, num_open, nodes, target, index);
	if (x-1>=0){
		affect_neighbour(x - 1, y, cost, num_open, nodes, target, index);
	}
	affect_neighbour(x, y + 1, cost, num_open, nodes, target, index);
	if (y-1>=0){
		affect_neighbour(x, y - 1, cost, num_open, nodes, target, index);
	}
}

// Open/update neighbours
void affect_neighbour(u8 x, u8 y, u12 cost, int* num_open, node_t* nodes,
		point_t* target, u12 parent_index) {

	int i = xy_to_i(x, y);

	// If node already opened
	if (nodes[i].set > 0) {
		if (nodes[i].cost > (cost + 1)) {
			nodes[i].prev = parent_index;
			nodes[i].cost = cost + 1;
		}
	} else if (is_openable(x, y, nodes)) {
		is_openable(x, y, nodes);
		open_node(x, y, cost, num_open, nodes, target, parent_index);
	}
}

// Check if a node is openable
u1 is_openable(u8 x, u8 y, node_t* nodes) {
	if ((x > grid_size - 1) | (y > grid_size - 1)) {
		return 0;
	}
	// If off grid (negative)
	if (x < 0 | y < 0) {
		return 0;
	}
	if (is_wall(x, y)) {
		return 0;
	}
	return 1;
}

void open_node(u8 x, u8 y, u12 cost, int* num_open, node_t* nodes,
		point_t* target, u12 parent_index) {
	node_t n = { 2, cost + 1, parent_index };
	nodes[xy_to_i(x, y)] = n;
	num_open++;
}

// Verify is coordinate is in a wall
u1 is_wall(u8 x, u8 y) {
	for (uint32 i = 0; i < num_walls; i++) {
		// Horizontal
		if (walls[i].dir == 0) {
			int max_x = walls[i].x + walls[i].len - 1;
			if ((y == walls[i].y) & (x >= walls[i].x) & (x <= max_x)) {
				return 1;
			}
		} else {
			int max_y = walls[i].y + walls[i].len - 1;
			if ((x == walls[i].x) & (y >= walls[i].y) & (y <= max_y)) {
				return 1;
			}
		}
	}
	return 0;
}


// Returns the index of the best node by n.cost + manhattan distance
int get_best_open_node(point_t target, node_t* nodes) {
	int index = -1;
	uint32 min_cost = 2147483647;
	int i;
	for (i = 0; i < NUM_NODES; i++) {
		if (nodes[i].set == 2) {
			uint32 node_cost = manhattan(i_to_x(i), i_to_y(i), &target) + nodes[i].cost;

			if (node_cost < min_cost) {
				index = i;
				min_cost = node_cost;
			}
		}
	}
	return index;
}

void close_node(int index, int* num_open, node_t* nodes) {
	nodes[index].set = 1;
	num_open--;
}

int manhattan(u8 x, u8 y, point_t* point2) {
	return abs(x - point2->x) + abs(y - point2->y);
}
