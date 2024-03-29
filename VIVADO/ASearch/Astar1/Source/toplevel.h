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
typedef ap_uint<12> u12;
typedef ap_uint<8> u8;
typedef ap_uint<4> u4;
typedef ap_uint<2> u2;
typedef ap_uint<1> u1;

//Prototypes
void toplevel(hls::stream<uint32> &input, hls::stream<uint32> &output);
void swap(u4 *x, u4 *y);
void permute(u12 matrix[][12], u4 tour[13], u4 size, u16* smallest_distance, u4 best_tour[13]);
u16 get_shortest_loop(u12 matrix[][12], u4 best_tour[13], u4 size, u4 tour[13]);
u16 get_path_cost(u12 matrix[][12], u4 tour[13],  u4 size);

typedef struct {
	u8 x;
	u8 y;
	u1 dir;
	u8 len;
} wall_t;

typedef struct {
	u8 x;
	u8 y;
} point_t;

typedef struct {
	u2 set;
	u12 cost;
	u12 prev;
} node_t;


#endif
