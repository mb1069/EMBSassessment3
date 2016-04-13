#include "toplevel.h"
//Input data storage
int32 inputdata[NUMDATA];

//Prototypes
int32 addall(int32 *data);
int32 subfromfirst(int32 *data);

//Top-level function
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output) {
#pragma HLS RESOURCE variable=input core=AXI4Stream
#pragma HLS RESOURCE variable=output core=AXI4Stream
#pragma HLS INTERFACE ap_ctrl_none port=return

    //Read in NUMDATA items
    readloop: for(int i = 0; i < NUMDATA; i++) {
        inputdata[i] = input.read();
    }

    world_t world;
    memcpy(&inputdata, &world, sizeof(world));
	printf("Received world: \n\r");
	printf("    - ID: %d \n\r", (short int) world.id);
	printf("    - Size: %d \n\r", (short int) world.size);
	printf("    - Height: %d \n\r",(short int) world.height);
	printf("    - Width: %d \n\r",(short int) world.width);
	printf("    - Num Waypoints: %d \n\r",(short int) world.num_waypoints);
	printf("    - Num Walls: %d \n\r",(short int) world.num_walls);


    int32 val1 = addall(inputdata);
    output.write(val1);
    int32 val2 = subfromfirst(inputdata);
    output.write(val2);

    return 0;
}

int32 addall(int32 *data) {
    int32 total = 0;
    addloop: for(int i = 0; i < NUMDATA; i++) {
        total = total + data[i];
    }
    return total;
}

int32 subfromfirst(int32 *data) {
    int32 total = data[0];
    subloop: for(int i = 1; i < NUMDATA; i++) {
        total = total - data[i];
    }
    return total;
}
