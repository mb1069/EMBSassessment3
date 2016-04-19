#include "toplevel.h"


int main() {
	int i;

    hls::stream<uint32> to_hw, from_hw;

// Small 0
//    u8 size = 10;
//    u8 waypoints[4][2] = {
//    		{2, 0},
//			{7, 2},
//			{2, 3},
//			{6, 5}
//    };
//
//    u8 walls[6][4] = {
//    		{0, 8, 0, 1},
//			{5, 3, 1, 3},
//			{9, 4, 1, 2},
//			{3, 2, 0 ,4},
//    		{4, 5, 1, 4},
//			{3, 8, 1, 1}
//    };

    // Med 0
//    u8 size = 30;
//	u8 waypoints[10][2] = {
//        		{22, 12},
//    			{19, 27},
//    			{26, 6},
//    			{27, 16},
//				{28, 17},
//				{3, 7},
//				{17, 28},
//				{25, 18},
//				{18, 27},
//				{1, 8}
//	};
//
//	u8 walls[12][4] = {
//		{0, 28, 0, 5},
//		{5, 23, 1, 9},
//		{9, 14, 1, 6},
//		{23, 22, 0, 9},
//		{4, 15, 1, 6},
//		{13, 18, 1, 3},
//		{2, 10, 1, 7},
//		{12, 23, 0, 3},
//		{17, 17, 1, 6},
//		{15, 23, 0, 5},
//		{20, 25, 0, 9},
//		{28, 21, 0, 2}
//  };



// Large 0
    u8 size = 60;
    u8 waypoints[12][2] =  {
    		{7, 33},
			{57, 49},
			{45, 12},
			{49, 43},
			{21, 1},
			{3, 40},
			{48, 54},
			{56, 43},
			{57, 42},
			{45, 51},
			{56, 45},
			{59, 53}
    };

    u8 walls[20][4] =  {
    		{0, 28, 0, 8},
    		{35, 53, 1, 6},
    		{39, 14, 1, 19},
    		{53, 22, 0, 18},
    		{4, 15, 1, 19},
    		{43, 48, 1, 4},
    		{32, 40, 1, 4},
    		{12, 23, 0, 11},
    		{17, 47, 1, 15},
    		{45, 53, 0, 8},
    		{20, 55, 0, 9},
    		{58, 51, 0, 1},
    		{52, 42, 1, 4},
    		{56, 6, 0, 15},
    		{28, 17, 1, 9},
    		{47, 28, 0, 8},
    		{48, 57, 1, 2},
    		{58, 18, 0, 5},
    		{17, 11, 0, 1},
    		{17, 42, 0, 1},
    };

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


    uint32 testdata2[27] = {
    		0x003C0C14,
			0x31392107,
			0x2B310C2D,
			0x28030115,
			0x2B383630,
			0x332D2A39,
			0x353B2D38,
			0x08001C00,
			0x06013523,
			0x13010E27,
			0x12001635,
			0x13010F04,
			0x0401302B,
			0x04012820,
			0x0B00170C,
			0x0F012F11,
			0x0800352D,
			0x09003714,
			0x0100333A,
			0x04012A34,
			0x0F000638,
			0x0901111C,
			0x08001C2F,
			0x02013930,
			0x0500123A,
			0x01000B11,
			0x01002A11};

    uint32 testdata3[27] = {0x003C0C14, 0x30050D1E, 0x0836230D, 0x24261006,
    		0x08353B3B, 0x02060029, 0x291F1919, 0x0A001230, 0x0C001535,
			0x0700361B, 0x01003A0C, 0x13010034, 0x0C00392D, 0x07011917,
			0x0F00260C, 0x07002910, 0x0E00362B, 0x06013930, 0x0F012F13,
			0x0F010613, 0x01010E30, 0x0E001D37, 0x0300030C, 0x0E00371B,
			0x12001104, 0x08002C15, 0x02002B15};

    //Write input data
    printf("Num data: %d \r", (int) (num_walls+(num_waypoints/2)+1));
    for (i = 0; i < num_walls+(num_waypoints/2)+1; i++) {
        to_hw.write(testdata2[i]);
    }

    //Run the hardware
    toplevel(to_hw, from_hw);
    //Read and report the output
    uint32 sub = from_hw.read();
    uint32 result[400];
    i = 0;
    while(sub!='\n'){
    	result[i++] = sub;
    	sub = from_hw.read();
    }

    printf("\n\r");
    for (int z = 0; z < i; z++){
    	int x = (result[z] >> 16) & 0xFF;
    	int y = (result[z]) & 0xFF;
    	printf("%08x ", (int) result[z]);
//    	printf("x: %d   %04x y: %d   %08x \r", x, (int) (result[z] >> 16) & 0xFF, y, (int) result[z]);
    }
    printf("\n\r");
    printf("Shortest path: %d\n", (int) from_hw.read());

}
