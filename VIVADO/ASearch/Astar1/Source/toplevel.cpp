#include "toplevel.h"

//Prototypes
void close_node(int index, int* num_open, node_t* nodes);
int get_best_node(point_t target, node_t* nodes);
int manhattan(point_t* point1, point_t* point2);
int compare_coords(point_t p1, point_t p2);
void check_neighbours(node_t* n, int* num_open, node_t* nodes, point_t* target);
int is_openable(int x, int y, node_t* nodes);
int is_wall(int x, int y);
int is_waypoint(int x, int y);
int find_first_empty_slot(node_t* nodes, point_t* target);
void open_node(int x, int y, int cost, int* num_open, node_t* nodes, point_t* target);
void affect_neighbour(int x, int y, int cost, int* num_open, node_t* nodes, point_t* target);
int is_in_set(int x, int y, node_t nodes[]);
u16 get_shortest_path(point_t w1,  point_t w2);

int NUMDATA = 27;


wall_t walls[20];
uint32 num_walls;
uint32 read_walls = 0;

point_t waypoints[12];
uint32 num_waypoints;
uint32 read_waypoints = 0;

u16 solutions[12][12];


u8 grid_size;

int NUM_NODES = 2000;
node_t nodes[2000];

//Top-level function
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output) {
#pragma HLS RESOURCE variable=input core=AXI4Stream
#pragma HLS RESOURCE variable=output core=AXI4Stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	int i;
    //Read in NUMDATA items
    readloop: for(i = 0; i < NUMDATA; i++) {
        int32 in = input.read();
        if (i==0){
        	grid_size     = (in >> 16) & 0xFF;
        	num_waypoints = (in >> 8) & 0xFF;
        	num_walls     =  in & 0xFF;
        } else if (read_waypoints<num_waypoints){
        	waypoints[read_waypoints].x = in & 0xFF;
        	waypoints[read_waypoints].y = (in >> 8) & 0xFF;
        	read_waypoints++;
        	if (read_waypoints<num_waypoints){
				waypoints[read_waypoints].x = (in >> 16) & 0xFF;
				waypoints[read_waypoints].y = (in >> 24) & 0xFF;
				read_waypoints++;
        	}
        } else if (read_walls<num_walls){
        	walls[read_walls].x   = in & 0xFF;
        	walls[read_walls].y   = (in >> 8) & 0xFF;
        	walls[read_walls].dir = (in >> 16) & 0xFF;
        	walls[read_walls].len = (in >> 24) & 0xFF;
        	read_walls++;
        }
        if (read_walls==num_walls){
        	break;
        }
    }
    printf("\n\r");

    // Print confirmation that data is correctly received

    printf("Grid_size: %d \n", (int) grid_size);

    printf("Num_waypoints: %d \n", (int) num_waypoints);

    for (i=0; i<num_waypoints; i++){
    	printf("Waypoint %d  x: %d y: %d \n\r", i, (int) waypoints[i].x, (int) waypoints[i].y);
    }

    printf("Num_walls: %d \n", (int) num_walls);
    for (i=0; i<num_walls; i++){
    	printf("Walls %d  x: %d y: %d dir: %d len: %d \n\r", i, (int) walls[i].x,(int) walls[i].y,(int) walls[i].dir,(int) walls[i].len);
    }

    for (int x=0; x<grid_size; x++){
    	for (int y=0; y<grid_size; y++){
    		if (is_wall(y,x)){
    			printf("@");
    		} else if (is_waypoint(y,x)){
    			printf("x");
    		} else {
    			printf(".");
    		}
    	}
    	printf("\r");
    }

    // Set active, 0 cost and starting waypoint
    for (u8 w1 = 0; w1<num_waypoints; w1++){
    	for (u8 w2 = w1; w2<num_waypoints; w2++){
    		if (w1!=w2){
    			printf("Going from %d,%d to %d,%d (%d,%d) \n\r",(int) waypoints[w1].x, (int) waypoints[w1].y, (int) waypoints[w2].x, (int) waypoints[w2].y, (int) w1, (int) w2);
    			if ((w1==4) & (w2==11)){
    				printf("Debug time!\n\r");
    			}
    			solutions[w1][w2] = get_shortest_path(waypoints[w1], waypoints[w2]);
    			solutions[w2][w1] = solutions[w1][w2];
    			printf("Solution: %d \n\r", (int) get_shortest_path(waypoints[w1], waypoints[w2]));
    			printf("\n\r");
    		}
    	}
    }
    for (u8 w1 = 0; w1<num_waypoints; w1++){
    	for (u8 w2 = 0; w2<num_waypoints; w2++){
    		printf("%d ", (int) (solutions[w1][w2]));
    	}
    	printf("\r");
    }


    output.write(0);

    return;
}

u16 get_shortest_path(point_t w1,  point_t w2){

	for(int n=0; n<NUM_NODES; n++){
		nodes[n].set=0;
	}
	u16 min_len = 0;
    node_t initial;
    initial.set = 2;
    initial.cost = 1;
    initial.coords = w1;

    nodes[0] = initial;
    int num_open = 1;
    while (num_open > 0){
    	int index = get_best_node(w2, nodes);
    	int x = nodes[index].coords.x;
    	int y = nodes[index].coords.y;
    	if (compare_coords(nodes[index].coords, w2)){
    		min_len = nodes[index].cost;
    		break;
    	} else {
    		check_neighbours(&nodes[index], &num_open, nodes, &w2);
    	}
    	close_node(index, &num_open, nodes);
    }
    return min_len;
}

void check_neighbours(node_t* n, int* num_open, node_t* nodes, point_t* target){
	int x = n->coords.x;
	int y = n->coords.y;
	affect_neighbour(x+1, y, n->cost, num_open, nodes, target);
	affect_neighbour(x-1, y, n->cost, num_open, nodes, target);
	affect_neighbour(x, y+1, n->cost, num_open, nodes, target);
	affect_neighbour(x, y-1, n->cost, num_open, nodes, target);
}

void affect_neighbour(int x, int y, int cost, int* num_open, node_t* nodes, point_t* target){
	int i = is_in_set(x,y, nodes);
	if (i>0){
		if (nodes[i].cost>(cost+1)){
			nodes[i].cost = cost+1;
		}
	} else if (is_openable(x,y, nodes)){
		open_node(x, y, cost, num_open, nodes, target);
	}
}

void open_node(int x, int y, int cost, int* num_open, node_t* nodes, point_t* target){
	int i = find_first_empty_slot(nodes, target);
	point_t p = {x,y};
	node_t n = {2, cost+1, p};
	nodes[i] = n;
	num_open++;
}

int is_openable(int x, int y, node_t* nodes){
	// If off grid (positive)
	if ((x>grid_size-1) | (y>grid_size-1)){
		return 0;
	}
	// If off grid (negative)
	if (x<0 | y<0){
		return 0;
	}
	// If is wall or node already explored
	int i = is_in_set(x, y, nodes);
	if (is_wall(x, y) | (i>0)){
		return 0;
	}
	return 1;
}

int is_in_set(int x, int y, node_t nodes[]){
	for (int i=0; i<NUM_NODES; i++){
		if (nodes[i].coords.x==x){
			if (nodes[i].coords.y==y){
				if (nodes[i].set!=0){
					return i;
				}
			}
		}
	}
	return 0;
}

int is_waypoint(int x, int y){
	for (int i=0; i<num_waypoints; i++){
		if ((waypoints[i].x==x) && (waypoints[i].y==y)){
			return 1;
		}
	}
	return 0;
}



int is_wall(int x, int y){
	for (int i=0; i<num_walls; i++){
		// Horizontal
		if (walls[i].dir==0){
			int max_x = walls[i].x+walls[i].len-1;
			if ((y==walls[i].y) & (x>=walls[i].x) & (x<=max_x)){
				return 1;
			}
		} else {
			int max_y = walls[i].y+walls[i].len-1;
			if ((x==walls[i].x) & (y>=walls[i].y) & (y<=max_y)){
				return 1;
			}
		}
	}
	return 0;
}

int compare_coords(point_t p1, point_t p2){
	if ((p1.x==p2.x) && (p1.y==p2.y)){
		return 1;
	}
	return 0;
}


// Returns the index of the best node by n.cost + manhattan distance
int get_best_node(point_t target, node_t* nodes){
	int index = -1;
	uint32 min_cost = 2147483647;
	int i;
	for (i=0; i<NUM_NODES; i++){
//		printf("Candidate node i:%d x:%d y:%d set:%d cost:%d \n\r", (int) i, (int) nodes[i].coords.x, (int) nodes[i].coords.y, (int) nodes[i].set, (int) nodes[i].cost);
		if (nodes[i].set==2){
			uint32 node_cost = manhattan(&nodes[i].coords, &target) + nodes[i].cost;
			if (node_cost<min_cost){
				index = i;
				min_cost = node_cost;
			}
		}
		if (nodes[i].set==0){
			break;
		}
	}
	if (index==-1){
		//printf("Best node not found!\n\r");
	}
	return index;
}

void close_node(int index, int* num_open, node_t* nodes){
	nodes[index].set = 1;
//	int i = find_first_empty_slot(closed);
//	closed[i].set = 1;
//	closed[i].cost = 0;
//	closed[i].coords = open[index].coords;
	num_open--;
}

int find_first_empty_slot(node_t* nodes, point_t* target){
	int i = 0;
	for (i=0; i<NUM_NODES; i++){
		if (nodes[i].set==0){
			return i;
		}
	}

	int c = 0;
	int index = -1;
	for (i=0; i<NUM_NODES; i++){
		u16 node_val = nodes[i].cost + manhattan(&nodes[i].coords, target);
		if (node_val>c){
			c = node_val;
			index = i;
		}
	}

	return index;
}



int manhattan(point_t* point1, point_t* point2){
	return abs(point1->x - point2->x) + abs(point1->y - point2->y);
}
