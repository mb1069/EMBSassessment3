#include "toplevel.h"


int main() {
	int i;

    hls::stream<uint32> to_hw, from_hw;

// Small 0
    u8 size = 10;
    u8 waypoints[4][2] = {
    		{2, 0},
			{7, 2},
			{2, 3},
			{6, 5}
    };

    u8 walls[6][4] = {
    		{0, 8, 0, 1},
			{5, 3, 1, 3},
			{9, 4, 1, 2},
			{3, 2, 0 ,4},
    		{4, 5, 1, 4},
			{3, 8, 1, 1}
    };



// Large 0
//    u8 size = 60;
//    u8 waypoints[12][2] =  {
//    		{7, 33},
//			{57, 49},
//			{45, 12},
//			{49, 43},
//			{21, 1},
//			{3, 40},
//			{48, 54},
//			{56, 43},
//			{57, 42},
//			{45, 51},
//			{56, 45},
//			{59, 53}
//    };
//
//    u8 walls[20][4] =  {
//    		{0, 28, 0, 8},
//    		{35, 53, 1, 6},
//    		{39, 14, 1, 19},
//    		{53, 22, 0, 18},
//    		{4, 15, 1, 19},
//    		{43, 48, 1, 4},
//    		{32, 40, 1, 4},
//    		{12, 23, 0, 11},
//    		{17, 47, 1, 15},
//    		{45, 53, 0, 8},
//    		{20, 55, 0, 9},
//    		{58, 51, 0, 1},
//    		{52, 42, 1, 4},
//    		{56, 6, 0, 15},
//    		{28, 17, 1, 9},
//    		{47, 28, 0, 8},
//    		{48, 57, 1, 2},
//    		{58, 18, 0, 5},
//    		{17, 11, 0, 1},
//    		{17, 42, 0, 1},
//    };

    // Max size of (20 walls) + (12 waypoints /2) + 1 info packet
    uint32 testdata[27];


    u8 num_waypoints = sizeof(waypoints)/sizeof(waypoints[0]);
    u8 num_walls = sizeof(walls)/sizeof(walls[0]);

    uint32 info = 0;
    info |= ((uint32) size) << 16;
    info |= ((uint32) num_waypoints) << 8;
    info |= ((uint32) num_walls);
    testdata[0] = info;

    // Data being copied over:
    // 4 bytes of waypoints (8, 8, 8, 8) in 1 32
    // 1 32 bit for total length
    // 4 bytes per wall * number of walls
    memcpy(&testdata[1],&waypoints,  sizeof(waypoints));


    memcpy(&testdata[1+(num_waypoints/2)], &walls, sizeof(walls[0])*num_walls);

    for (i = 0; i<num_walls+(num_waypoints/2)+1; i++){
    	printf("%08x ", (int) testdata[i]);
    }
    printf("\r");


    //Write input data
    printf("Num data: %d \r", (int) (num_walls+(num_waypoints/2)+1));
    for (i = 0; i < num_walls+(num_waypoints/2)+1; i++) {
        to_hw.write(testdata[i]);
    }

    //Run the hardware
    toplevel(to_hw, from_hw);
    //Read and report the output
    uint32 sub = from_hw.read();
    printf("Shortest path: %04x\n", (int) from_hw.read());

}
