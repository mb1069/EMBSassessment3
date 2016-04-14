#include "toplevel.h"

//Prototypes
void close_node(int index, int* num_open);
int get_best_node();
int manhattan(point_t* point1, point_t* point2);
int compare_coords(point_t p1, point_t p2);
void check_neighbours(node_t* n, int* num_open);
int is_openable(int x, int y);
int is_wall(int x, int y);
int find_first_empty_slot(node_t* nodes);
void open_node(int x, int y, int n, int* num_open);
void affect_neighbour(int x, int y, int cost, int* num_open);
int is_in_set(int x, int y, node_t nodes[]);

int NUMDATA = 22;
point_t waypoints[2];

uint32 num_walls;
wall_t walls[20];
u8 grid_size;
//Input data storage
int32 inputdata[22];

int NUM_NODES = 2000;
node_t closed[2000];
node_t open[2000];





//Top-level function
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output) {
#pragma HLS RESOURCE variable=input core=AXI4Stream
#pragma HLS RESOURCE variable=output core=AXI4Stream
#pragma HLS INTERFACE ap_ctrl_none port=return

	int i;
	int wall_num = 0;
    //Read in NUMDATA items
    readloop: for(i = 0; i < NUMDATA; i++) {
        int32 in = input.read();
        if (i==0){
        	waypoints[0].x = in & 0xFF;
        	waypoints[0].y = (in >> 8) & 0xFF;
        	waypoints[1].x = (in >> 16) & 0xFF;
        	waypoints[1].y = (in >> 24) & 0xFF;
        } else if (i==1){
        	num_walls = (in>>16) & 0xFF;
        	grid_size = in & 0xFF;
        	NUMDATA = num_walls+2;
        } else if (i>1){
        	walls[wall_num].x   = in & 0xFF;
        	walls[wall_num].y   = (in >> 8) & 0xFF;
        	walls[wall_num].dir = (in >> 16) & 0xFF;
        	walls[wall_num].len = (in >> 24) & 0xFF;
        	wall_num++;
        }
    }

    printf("Grid_size: %d \n\r", (int) grid_size);

    printf("Num_walls: %d \n\r", (int) num_walls);

    for (i=0; i<2; i++){
    	printf("Waypoint %d  x: %d y: %d \n\r", i, (int) waypoints[i].x, (int) waypoints[i].y);
    }
    for (i=0; i<num_walls; i++){
    	printf("Walls %d  x: %d y: %d dir: %d len: %d \n\r", i, (int) walls[i].x,(int) walls[i].y,(int) walls[i].dir,(int) walls[i].len);
    }

    int min_len = 0;
    // Set active, 0 cost and starting waypoint
    node_t initial;
    initial.set = 1;
    initial.cost = 1;
    initial.coords = waypoints[0];
    point_t origin = {0,0};


    open[0] = initial;
    int num_open = 1;

    for (int x=0; x<grid_size; x++){
    	for (int y=0; y<grid_size; y++){
    		printf("%d", (int) is_wall(y,x));
    	}
    	printf("\r");
    }

    while (num_open > 0){
    	int index = get_best_node();
    	//printf("x: %d y: %d num_open: %d \n\r", (int) open[index].coords.x, (int) open[index].coords.y, (int) num_open);
    	if (compare_coords(open[index].coords, waypoints[1])){
    		min_len = open[index].cost;
    		break;
    	} else {
    		check_neighbours(&open[index], &num_open);
    	}
    	close_node(index, &num_open);

        for (int x=0; x<grid_size; x++){
        	for (int y=0; y<grid_size; y++){
        		if (is_wall(y,x)){
        			printf("2");
        		} else if (is_in_set(y,x,open) | is_in_set(y,x,closed)){
        			printf("1");
        		} else {
        			printf("0");
        		}
        	}
        	printf("\r");
        }
        printf("\n\r");

    }
    min_len--;

    printf("Got solution: %d \n\r", (int) min_len);

    output.write(min_len);

    return;
}

void check_neighbours(node_t* n, int* num_open){
	int x = n->coords.x;
	int y = n->coords.y;
	affect_neighbour(x+1, y, n->cost, num_open);
	affect_neighbour(x-1, y, n->cost, num_open);
	affect_neighbour(x, y+1, n->cost, num_open);
	affect_neighbour(x, y-1, n->cost, num_open);
}

void affect_neighbour(int x, int y, int cost, int* num_open){
	int i = is_in_set(x,y, open);
	if (i>0){
		if (open[i].cost>(cost+1)){
			open[i].cost = cost+1;
		}
	} else if (is_openable(x,y)){
		open_node(x, y, cost, num_open);
	}
}

void open_node(int x, int y, int cost, int* num_open){
	int i = find_first_empty_slot(open);
	point_t p = {x,y};
	node_t n = {1, cost+1, p};
	open[i] = n;
	num_open++;
}

int is_openable(int x, int y){
	// If off grid (positive)
	if ((x>grid_size-1) | (y>grid_size-1)){
		return 0;
	}
	// If off grid (negative)
	if (x<0 | y<0){
		return 0;
	}
	// If is wall or node already explored
	int i = is_in_set(x, y, closed);
	if (is_wall(x, y) | (i>0)){
		return 0;
	}
	return 1;
}

int is_in_set(int x, int y, node_t nodes[]){
	for (int i=0; i<NUM_NODES; i++){
		if (nodes[i].coords.x==x){
			if (nodes[i].coords.y==y){
				return i;
			}
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
int get_best_node(){
	int len = sizeof(open)/sizeof(open[0]);
	int index = -1;
	uint32 min_cost = 2147483647;
	for (int i=0; i<len; i++){
		if (open[i].set==1){
			uint32 node_cost = manhattan(&open[i].coords, &waypoints[1]) + open[i].cost;
			if (node_cost<min_cost){
				index = i;
				min_cost = node_cost;
			}
		}
	}
	if (index==-1){
		//printf("Best node not found!\n\r");
	}
	return index;
}

void close_node(int index, int*num_open){
	open[index].set = 0;
	int i = find_first_empty_slot(closed);
	closed[i].set = 1;
	closed[i].cost = 0;
	closed[i].coords = open[index].coords;
	num_open--;
}

int find_first_empty_slot(node_t* nodes){
	for (int i=0; i<NUM_NODES; i++){
		if (nodes[i].set==0){
			return i;
		}
	}
	printf("No open slots\n\r");
	return -1;
}



int manhattan(point_t* point1, point_t* point2){
	return abs(point1->x - point2->x) + abs(point1->y - point2->y);
}
