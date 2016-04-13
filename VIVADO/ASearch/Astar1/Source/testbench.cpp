#include "toplevel.h"


int main() {

    hls::stream<uint32> to_hw, from_hw;
    uint32 testdata[100];

    u8 waypoints[12][2] =  {
    		{7, 2},
			{2, 3},
			{6, 5},
    };

    u8 walls[20][4] =  {
    		{0, 8, 0, 1},
			{5, 3, 1, 3},
			{9, 4, 1, 2},
			{3, 2, 0, 4},
			{4, 5, 1, 4},
			{3, 8, 1, 1}
    };

    world_t world;
    world.id = 0;
    world.size = 0;
    world.height = 10;
    world.width = 10;
    world.num_waypoints = 3;
    world.num_walls = 6;
    world.start_x = 2;
    world.start_y = 9;

    memcpy(&testdata, &world, sizeof(world));

    //Create input data
    for(int i = 0; i < NUMDATA; i++) {
        testdata[i] = i;
    }
    testdata[0] = 2000;
    //Write input data
    for(int i = 0; i < NUMDATA; i++) {
        to_hw.write(testdata[i]);
    }

    //Run the hardware
    toplevel(to_hw, from_hw);

    //Read and report the output
    int sum = from_hw.read();
    int sub = from_hw.read();
    printf("Sum of input: %d\n", sum);
    printf("Values 1 to %d subtracted from value 0: %d\n", NUMDATA-1, sub);

    //Check values
    if(sum == 2780 && sub == 1220) {
        return 0;
    } else {
        return 1; //An error!
    }
}
