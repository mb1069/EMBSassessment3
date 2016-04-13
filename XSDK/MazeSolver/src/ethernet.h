
#include "xemaclite.h"
#include "main.h"

#ifndef ETHERNET_H_
#define ETHERNET_H_

void init_ethernet();
void request_world(int size, u64 id);
void receive_world(world_t* world);
void solve_world(world_t* world, u32 path_len);
int receive_reply();
#endif
