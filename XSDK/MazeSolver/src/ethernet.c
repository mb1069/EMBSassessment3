#include "xemaclite.h"
#include "ethernet.h"
#include "main.h"
XEmacLite ether;

static u8 mac_address[] = {0x00, 0x11, 0x22, 0x33, 0x00, 0x17}; // Remember to change this to *your* MAC address!


/*
 * Buffers used for Transmission and Reception of Packets. These are declared
 * as global so that they are not a part of the stack.
 */
u8 tmit_buffer[100];
u8 recv_buffer[100];

void init_ethernet(){
	//Initialise the driver
	XEmacLite_Config *etherconfig = XEmacLite_LookupConfig(XPAR_EMACLITE_0_DEVICE_ID);
	XEmacLite_CfgInitialize(&ether, etherconfig, etherconfig->BaseAddress);

	XEmacLite_SetMacAddress(&ether, mac_address); //Set our sending MAC address
}

void request_world(int size, u64 id) {
	XEmacLite_FlushReceive(&ether); //Clear any recieved messages

	int i;
	u8 *buffer = tmit_buffer;

	//Write the destination MAC address (broadcast in this case)
	*buffer++ = 0x00;
	*buffer++ = 0x11;
	*buffer++ = 0x22;
	*buffer++ = 0x44;
	*buffer++ = 0x00;
	*buffer++ = 0x50;

	//Write the source MAC address
	for(i = 0; i < 6; i++)
		*buffer++ = mac_address[i];

	//Write the type
	*buffer++ = 0x55;
	*buffer++ = 0xAB;

	//Write size
	*buffer++ = 0x01;
	*buffer++ = size;
	*buffer++ = (id & 0xFF000000) >> 24;
	*buffer++ = (id & 0xFF0000) >> 16;
	*buffer++ = (id & 0xFF00) >> 8;
	*buffer++ =  id & 0xFF;

	//Send the buffer
	//The size argument is the data bytes + XEL_HEADER_SIZE which is defined
	//as the size of the destination MAC plus the type/length field

	XEmacLite_Send(&ether, tmit_buffer, 10 + XEL_HEADER_SIZE);
}

//int memcmp2(u8 *arr1, u8 *arr2, int len){
//
//	while (len>=0){
//		xil_printf("Comparing: %d %d %d \n\r", (int) arr1[len], (int) arr2[len], len);
//		if (arr1[len]!=arr2[len]){
//			return 0;
//		}
//		len--;
//	}
//	return 1;
//}


void receive_world(world_t* world){
	int i;
	//Poll for receive packet. recv_len must be defined as volatile!
	volatile int recv_len = 0;
	while (recv_len == 0)  {
		recv_len = XEmacLite_Recv(&ether, recv_buffer);
		if (memcmp(recv_buffer, mac_address, 6)!=0){
			recv_len = 0;
		}
	}

	// We have a frame. Note that recv_len may be LONGER than the frame
	// that was sent. (Ethernet is weird, and that is why we need length
	// bytes in our protocols.)
	u8 *buffer = recv_buffer;

	for(i = 0; i < recv_len; i++) {
		// Loop until start of message
		if (*buffer++==0x55){
			if (*buffer++==0xAB){
				if (*buffer++==0x02){
					break;
				}
			}
		}
	}
	int world_id = *buffer++;
	for (i = 0; i < 3; i++){
		world_id = world_id << 8;
		world_id |= *buffer++;
	}

	world->width = *buffer++;
	world->height = *buffer++;

	world->num_waypoints = *buffer++;

	for (i = 0; i < world->num_waypoints; i++, buffer+=sizeof(point_t)){
		memcpy(&world->waypoints[i], buffer, sizeof(point_t));
	}

	world->num_walls = *buffer++;
	for (i = 0; i < world->num_walls; i++, buffer+=sizeof(wall_t)){
		memcpy(&world->walls[i], buffer, sizeof(wall_t));
//		world->walls[i].x = *buffer++; // X
//		world->walls[i].y = *buffer++; // Y
//		world->walls[i].dir = *buffer++; // Dir
//		world->walls[i].len = *buffer++; //Length
//		xil_printf("Wall %d %d %d %d \n\r", world->walls[i][0], world->walls[i][1] ,world->walls[i][2], world->walls[i][3]);
	}
}

void solve_world(world_t* world, u32 path_len){
	XEmacLite_FlushReceive(&ether); //Clear any recieved messages

	int i;
	u8 *buffer = tmit_buffer;

	//Write the destination MAC address (broadcast in this case)
	*buffer++ = 0x00;
	*buffer++ = 0x11;
	*buffer++ = 0x22;
	*buffer++ = 0x44;
	*buffer++ = 0x00;
	*buffer++ = 0x50;

	//Write the source MAC address
	for(i = 0; i < 6; i++)
		*buffer++ = mac_address[i];

	//Write the type
	*buffer++ = 0x55;
	*buffer++ = 0xAB;

	//Write size
	*buffer++ = 0x03;
	*buffer++ = world->size;
	*buffer++ = (world->id & 0xFF000000) >> 24;
	*buffer++ = (world->id & 0xFF0000) >> 16;
	*buffer++ = (world->id & 0xFF00) >> 8;
	*buffer++ = (world->id & 0xFF);
	*buffer++ = 0;
	*buffer++ = (path_len & 0xFF000000) >> 24;
	*buffer++ = (path_len & 0xFF0000) >> 16;
	*buffer++ = (path_len & 0xFF00) >> 8;
	*buffer++ = (path_len & 0xFF);
	xil_printf("Sent solution\n\r");

	//Send the buffer
	//The size argument is the data bytes + XEL_HEADER_SIZE which is defined
	//as the size of the destination MAC plus the type/length field
	XEmacLite_Send(&ether, tmit_buffer, 11 + XEL_HEADER_SIZE);
}

int receive_reply(){
	//Poll for receive packet. recv_len must be defined as volatile!
	volatile int recv_len = 0;
	while (recv_len == 0)  {
		recv_len = XEmacLite_Recv(&ether, recv_buffer);
		if (memcmp(recv_buffer, mac_address, 6)!=0){
			recv_len = 0;
		}
	}

	// We have a frame. Note that recv_len may be LONGER than the frame
	// that was sent. (Ethernet is weird, and that is why we need length
	// bytes in our protocols.)
	xil_printf("Received mark \n\r");


	return recv_buffer[15];
}
