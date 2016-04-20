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

    uint32 testdata3[27] = {0x001E0A0C, 0x1B130C16, 0x101B061A, 0x0703111C, 0x12191C11, 0x08011B12, 0x05001C00, 0x09011705, 0x06010E09, 0x09001617, 0x06010F04, 0x0301120D, 0x07010A02, 0x0300170C, 0x06011111, 0x0500170F, 0x09001914, 0x0200151C};
    int num_written = 0;
    //Write input data
    printf("Num data: %d \r", (int) (num_walls+(num_waypoints/2)+1));
    for (i = 0; i < num_walls+(num_waypoints/2)+1; i++) {
        to_hw.write(testdata[i]);
        num_written++;
    }
    num_written++;

    printf("Sent data\n\r");
    //Run the hardware
    toplevel(to_hw, from_hw);
    //Read and report the output
    uint32 shortest_path = from_hw.read();


    uint32 res = from_hw.read();
    while (res != (uint32) 0xFFFF){
    	uint32 x = (res >> 16) & 0xFF;
    	uint32 y = (res) & 0xFF;
    	printf("x: %d y: %d \n\r ", (long) x, (long) y);
    	res = from_hw.read();
    }

    printf("Num written: %d \n\r", (int) num_written);
    printf("Shortest path: %d\n", (int) shortest_path);
    printf("Num written: %d \n\r", (int) num_written);


//    num_written = 0 ;
//    for (i = 0; i < num_walls+(num_waypoints/2)+1; i++) {
//        to_hw.write(testdata[i]);
//        num_written++;
//    }
//    num_written++;
//    printf("Sent data\n\r");
//    //Run the hardware
//    toplevel(to_hw, from_hw);
//    //Read and report the output
//    shortest_path = from_hw.read();
//
//    res = from_hw.read();
//    while (res != (uint32) 0xFFFF){
//    	int x = (res >> 16) & 0xFF;
//    	int y = (res) & 0xFF;
//    	printf("x: %d y: %d \n\r ", x, y);
//    	res = from_hw.read();
//    }
//
//    printf("Num written: %d \n\r", (int) num_written);
//    printf("Shortest path: %d\n", (int) shortest_path);
//    printf("From %d to %d \n\r", from_hw.empty(), to_hw.empty());
//
//    num_written = 0 ;
//    for (i = 0; i < num_walls+(num_waypoints/2)+1; i++) {
//        to_hw.write(testdata[i]);
//        num_written++;
//    }
//    num_written++;
//    printf("Sent data\n\r");
//    //Run the hardware
//    toplevel(to_hw, from_hw);
//    //Read and report the output
//    shortest_path = from_hw.read();
//
//    res = from_hw.read();
//    while (res != (uint32) 0xFFFF){
//    	int x = (res >> 16) & 0xFF;
//    	int y = (res) & 0xFF;
////    	printf("%08x ", (int) res);
//    	res = from_hw.read();
//    }
//
//    printf("Num written: %d \n\r", (int) num_written);
//    printf("Shortest path: %d\n", (int) shortest_path);
//    printf("From %d to %d \n\r", from_hw.empty(), to_hw.empty());

}
