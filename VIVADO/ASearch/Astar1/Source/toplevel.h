#ifndef __TOPLEVEL_H_
#define __TOPLEVEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <ap_int.h>
#include <hls_stream.h>

//Typedefs
typedef ap_uint<32> uint32;
typedef ap_int<32> int32;
typedef ap_uint<8> u8;

//Prototypes
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output);

//Size of the example functionality
#define NUMDATA 100

typedef struct  __attribute__((packed)){
	u8 id;
	u8 size;
	u8 width;
	u8 height;
	u8 num_waypoints;
	u8 waypoints[12][2];
	u8 num_walls;
	u8 walls[20][4];
	u8 start_x;
	u8 start_y;
} world_t;


#endif
