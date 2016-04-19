#include "toplevel.h"

//Prototypes
void close_node(int index, int* num_open, node_t* nodes);
int get_best_node(point_t target, node_t* nodes);
int manhattan(point_t* point1, point_t* point2);
u1 compare_coords(point_t p1, point_t p2);
void check_neighbours(node_t* n, int* num_open, node_t* nodes, point_t* target,
		int index);
u1 is_openable(int x, int y, node_t* nodes);
u1 is_wall(int x, int y);
u1 is_waypoint(int x, int y);
int find_first_empty_slot(node_t* nodes, point_t* target);
void open_node(int x, int y, int cost, int* num_open, node_t* nodes,
		point_t* target, int index);
void affect_neighbour(int x, int y, int cost, int* num_open, node_t* nodes,
		point_t* target, int index);
int is_in_set(int x, int y, node_t nodes[]);
u16 get_shortest_path(point_t w1, point_t w2, u1 get_path,
		hls::stream<uint32> &output);

int NUMDATA = 27;

wall_t walls[20];
uint32 num_walls = 1;
uint32 read_walls = 0;

point_t waypoints[12];
u4 num_waypoints;
uint32 read_waypoints = 0;

u16 distance_matrix[12][12];

u4 best_tour[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

u8 grid_size;

int NUM_NODES = 3600;
node_t nodes[3600];

//Top-level function
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output) {
#pragma HLS RESOURCE variable=input core=AXI4Stream
#pragma HLS RESOURCE variable=output core=AXI4Stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	int i;
	//Read in NUMDATA items
	int32 in = input.read();
	grid_size = (in >> 16) & 0xFF;
	num_waypoints = (in >> 8) & 0xFF;
	num_walls = in & 0xFF;
	printf("Num_waypoints: %d \n", (int) num_waypoints);
	printf("Num_walls: %d \n", (int) num_walls);

	readloop: for (i = 0; i < (num_waypoints/2) + (num_walls); i++) {
		in = input.read();
		if (read_waypoints < num_waypoints) {
			waypoints[read_waypoints].x = in & 0xFF;
			waypoints[read_waypoints].y = (in >> 8) & 0xFF;
			read_waypoints++;
			if (read_waypoints < num_waypoints) {
				waypoints[read_waypoints].x = (in >> 16) & 0xFF;
				waypoints[read_waypoints].y = (in >> 24) & 0xFF;
				read_waypoints++;
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
	}
	printf("\n\r");

	// Print confirmation that data is correctly received

	printf("Grid_size: %d \n", (int) grid_size);

	for (i = 0; i < num_waypoints; i++) {
		printf("Waypoint %d  x: %d y: %d \r", i, (int) waypoints[i].x,
				(int) waypoints[i].y);
	}


	for (i = 0; i < num_walls; i++) {
		printf("Walls %d  x: %d y: %d dir: %d len: %d \r", i, (int) walls[i].x,
				(int) walls[i].y, (int) walls[i].dir, (int) walls[i].len);
	}

//    for (int x=0; x<grid_size; x++){
//    	for (int y=0; y<grid_size; y++){
//    		if (is_wall(y,x)){
//    			printf("@");
//    		} else if (is_waypoint(y,x)){
//    			printf("x");
//    		} else {
//    			printf(".");
//    		}
//    	}
//    	printf("\r");
//    }

// Set active, 0 cost and starting waypoint
	for (u8 w1 = 0; w1 < num_waypoints; w1++) {
		for (u8 w2 = w1; w2 < num_waypoints; w2++) {
			if (w1 != w2) {
//				printf("Going from %d,%d to %d,%d (%d,%d) \n\r",
//						(int) waypoints[w1].x, (int) waypoints[w1].y,
//						(int) waypoints[w2].x, (int) waypoints[w2].y, (int) w1,
//						(int) w2);
				distance_matrix[w1][w2] = get_shortest_path(waypoints[w1],
						waypoints[w2], 0, output);
				distance_matrix[w2][w1] = distance_matrix[w1][w2];
//				printf("Solution: %d \n\r", (int) distance_matrix[w1][w2]);
//				printf("\n\r");
			}
		}
	}
	for (u8 w1 = 0; w1 < num_waypoints; w1++) {
		for (u8 w2 = 0; w2 < num_waypoints; w2++) {
			printf("%d ", (int) (distance_matrix[w1][w2]));
		}
		printf("\r");
	}
	uint32 shortest_loop = get_shortest_loop(distance_matrix, num_waypoints,
			best_tour);
	printf("Best solution length: %d \n\r", (int) shortest_loop);
	printf("\n\rBest path: ");
	for (i = 0; i < num_waypoints; i++) {
		printf("%d, ", (int) best_tour[i]);
	}
	printf("\n\r");
	for (i = 0; i < num_waypoints; i++) {
		if (i != num_waypoints - 1) {
			printf("From %d -> %d \r", (int) best_tour[i],
					(int) best_tour[i + 1]);
			get_shortest_path(waypoints[best_tour[i]],
					waypoints[best_tour[i + 1]], 1, output);
		} else {
			printf("From %d -> %d \r", (int) best_tour[i], (int) best_tour[0]);
			get_shortest_path(waypoints[best_tour[i]], waypoints[best_tour[0]],
					1, output);
		}
	}
	output.write('\n');
	output.write(shortest_loop);
	return;
}

u16 get_shortest_path(point_t w1, point_t w2, u1 get_path,
		hls::stream<uint32> &output) {

	for (int n = 0; n < NUM_NODES; n++) {
		nodes[n].set = 0;
	}
	u11 min_len = 0;

	node_t initial;
	initial.set = 2;
	initial.cost = 1;
	initial.coords = w1;

	nodes[0] = initial;
	int num_open = 1;
	int index;
	while (num_open > 0) {
		index = get_best_node(w2, nodes);
		int x = nodes[index].coords.x;
		int y = nodes[index].coords.y;
		if (compare_coords(nodes[index].coords, w2)) {
			min_len = nodes[index].cost;
			break;
		} else {
			check_neighbours(&nodes[index], &num_open, nodes, &w2, index);
		}
		close_node(index, &num_open, nodes);
	}
	if (get_path) {
//		printf("From %d:%d -> %d:%d\r", (int) w1.x, (int) w1.y, (int) w2.x,
//				(int) w2.y);
		u1 flag = 1;
		while (flag) {
			uint32 out = 0;
			out = (0xFF & (uint32) nodes[index].coords.x) << 16;
			out |= 0xFF & (uint32) nodes[index].coords.y;
//        	int x = (out >> 16) & 0xFF;
//        	int y = (out) & 0xFF;
//        	printf("x: %d y: %d \r", x, y);
			output.write(out);
			if (index == nodes[index].prev) {
				flag = 0;
			} else {
				index = nodes[index].prev;
			}

		}
//		printf("\n");
		output.write((uint32) 0xFFFFFFFF);
	}
	return min_len - 1;

}

void check_neighbours(node_t* n, int* num_open, node_t* nodes, point_t* target,
		int index) {
	int x = n->coords.x;
	int y = n->coords.y;
	affect_neighbour(x + 1, y, n->cost, num_open, nodes, target, index);
	affect_neighbour(x - 1, y, n->cost, num_open, nodes, target, index);
	affect_neighbour(x, y + 1, n->cost, num_open, nodes, target, index);
	affect_neighbour(x, y - 1, n->cost, num_open, nodes, target, index);
}

void affect_neighbour(int x, int y, int cost, int* num_open, node_t* nodes,
		point_t* target, int index) {
	int i = is_in_set(x, y, nodes);
	if (i > 0) {
		if (nodes[i].cost > (cost + 1)) {
//			nodes[i].prev = index;
			nodes[i].cost = cost + 1;
		}
	} else if (is_openable(x, y, nodes)) {
		open_node(x, y, cost, num_open, nodes, target, index);
	}
}

void open_node(int x, int y, int cost, int* num_open, node_t* nodes,
		point_t* target, int index) {
	int i = find_first_empty_slot(nodes, target);
	point_t p = { x, y };
	node_t n = { 2, cost + 1, p, index };
	nodes[i] = n;
	num_open++;
}

u1 is_openable(int x, int y, node_t* nodes) {
	// If off grid (positive)
	if ((x > grid_size - 1) | (y > grid_size - 1)) {
		return 0;
	}
	// If off grid (negative)
	if (x < 0 | y < 0) {
		return 0;
	}
	// If is wall or node already explored
	int i = is_in_set(x, y, nodes);
	if (is_wall(x, y) | (i > 0)) {
		return 0;
	}
	return 1;
}

int is_in_set(int x, int y, node_t nodes[]) {
	for (int i = 0; i < NUM_NODES; i++) {
		if (nodes[i].coords.x == x) {
			if (nodes[i].coords.y == y) {
				if (nodes[i].set != 0) {
					return i;
				}
			}
		}
	}
	return 0;
}

u1 is_waypoint(int x, int y) {
	for (int i = 0; i < num_waypoints; i++) {
		if ((waypoints[i].x == x) && (waypoints[i].y == y)) {
			return 1;
		}
	}
	return 0;
}

u1 is_wall(int x, int y) {
	for (int i = 0; i < num_walls; i++) {
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

u1 compare_coords(point_t p1, point_t p2) {
	if ((p1.x == p2.x) && (p1.y == p2.y)) {
		return 1;
	}
	return 0;
}

// Returns the index of the best node by n.cost + manhattan distance
int get_best_node(point_t target, node_t* nodes) {
	int index = -1;
	uint32 min_cost = 2147483647;
	int i;
	for (i = 0; i < NUM_NODES; i++) {
//		printf("Candidate node i:%d x:%d y:%d set:%d cost:%d \n\r", (int) i, (int) nodes[i].coords.x, (int) nodes[i].coords.y, (int) nodes[i].set, (int) nodes[i].cost);
		if (nodes[i].set == 2) {
			uint32 node_cost = manhattan(&nodes[i].coords, &target)
					+ nodes[i].cost;
			if (node_cost < min_cost) {
				index = i;
				min_cost = node_cost;
			}
		}
		if (nodes[i].set == 0) {
			break;
		}
	}
	return index;
}

void close_node(int index, int* num_open, node_t* nodes) {
	nodes[index].set = 1;
	num_open--;
}

int find_first_empty_slot(node_t* nodes, point_t* target) {
	int i = 0;
	for (i = 0; i < NUM_NODES; i++) {
		if (nodes[i].set == 0) {
			return i;
		}
	}
//	int c = 0;
//	int index = -1;
//	for (i = 0; i < NUM_NODES; i++) {
//		u16 node_val = nodes[i].cost + manhattan(&nodes[i].coords, target);
//		if (node_val > c) {
//			c = node_val;
//			index = i;
//		}
//	}
//	printf("Kicked \n\r");
	return 0;
}

int manhattan(point_t* point1, point_t* point2) {
	return abs(point1->x - point2->x) + abs(point1->y - point2->y);
}
