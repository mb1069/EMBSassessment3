#ifndef __TOPLEVEL_H_
#define __TOPLEVEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <ap_int.h>
#include <hls_stream.h>

//Typedefs
typedef ap_uint<32> uint32;
typedef ap_int<32> int32;
typedef ap_uint<16> u16;
typedef ap_uint<8> u8;
typedef ap_uint<1> u1;

//Prototypes
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output);

typedef struct {
	u8 x;
	u8 y;
	u8 dir;
	u8 len;
} wall_t;

typedef struct {
	u8 x;
	u8 y;
} point_t;

typedef struct {
	u8 set;
	u16 cost;
	point_t coords;
} node_t;


#endif
