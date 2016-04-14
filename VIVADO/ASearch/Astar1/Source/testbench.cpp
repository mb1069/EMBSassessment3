#include "toplevel.h"


int main() {
	int i;
    hls::stream<uint32> to_hw, from_hw;


    u8 waypoints[2][2] =  {
			{2, 3},
			{6, 5}
    };

    u8 size = 10;

    u8 num_walls = 6;
    u8 walls[20][4] =  {
    		{0, 8, 0, 1},
			{5, 3, 1, 3},
			{9, 4, 1, 2},
			{3, 2, 0, 4},
			{4, 5, 1, 4},
			{3, 8, 1, 1}
    };

    // Max size of 22 * 32bits
    uint32 testdata[22];

    // Data being copied over:
    // 4 bytes of waypoints (8, 8, 8, 8) in 1 32
    // 1 32 bit for total length
    // 4 bytes per wall * number of walls
    memcpy(&testdata,&waypoints,  sizeof(waypoints));

    testdata[1] = (((uint32) num_walls) << 16) | (size);

    memcpy(&testdata[2], &walls, sizeof(walls[0])*num_walls);

    for (i = 0; i<22; i++){
    	printf("%08x ", (int) testdata[i]);
    }
    printf("\n\r");


    //Write input data
    for (i = 0; i < num_walls+2; i++) {
        to_hw.write(testdata[i]);
    }

    //Run the hardware
    toplevel(to_hw, from_hw);

    //Read and report the output
    int sub = from_hw.read();
    printf("Shortest path: %d\n", (int) from_hw.read());

}
