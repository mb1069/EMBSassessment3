#include "xemaclite.h"
#include "ethernet.h"
#include "main.h"
XEmacLite ether;

// My mac address
static u8 mac_address[] = { 0x00, 0x11, 0x22, 0x33, 0x00, 0x17 };

/*
 * Buffers used for Transmission and Reception of Packets. These are declared
 * as global so that they are not a part of the stack.
 */
u8 tmit_buffer[100];
u8 recv_buffer[100];

void init_ethernet() {
	//Initialise the driver
	XEmacLite_Config *etherconfig = XEmacLite_LookupConfig(
			XPAR_EMACLITE_0_DEVICE_ID);
	XEmacLite_CfgInitialize(&ether, etherconfig, etherconfig->BaseAddress);

	XEmacLite_SetMacAddress(&ether, mac_address); //Set our sending MAC address
}

/*
 * Method to request a world from the server
 */
void request_world(int size, u32 id) {
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
	for (i = 0; i < 6; i++)
		*buffer++ = mac_address[i];

	//Write the type
	*buffer++ = 0x55;
	*buffer++ = 0xAB;

	//Write size
	*buffer++ = 0x01;
	*buffer++ = size;
	memcpy(buffer, &id, 4);

	XEmacLite_Send(&ether, tmit_buffer, 10 + XEL_HEADER_SIZE);
}

/*
 * Method to compare two u8 arrays, returns 0 if len first elements in arrays are of equal value
 */
int memcmp2(u8 *buff1, u8 *buff2, int len) {
	int i;
	for (i = 0; i < len; i++) {
		if (buff1[i] != buff2[i]) {
			return 1;
		}
	}
	return 0;
}

/*
 * Method to receive world parameters including walls, waypoints, and dimensions
 */
void receive_world(world_t* world) {
	int i;
	//Poll for receive packet. recv_len must be defined as volatile!
	volatile int recv_len = 0;
	while (recv_len == 0) {
		recv_len = XEmacLite_Recv(&ether, recv_buffer);
		if (memcmp2(recv_buffer, mac_address, 6) != 0) {
			recv_len = 0;
		}
	}

	u8 *buffer = recv_buffer;
	for (i = 0; i < recv_len; i++) {
		// Loop until start of message
		if (*buffer++ == 0x55) {
			if (*buffer++ == 0xAB) {
				if (*buffer++ == 0x02) {
					break;
				}
			}
		}
	}
	int world_id = *buffer++;
	for (i = 0; i < 3; i++) {
		world_id = world_id << 8;
		world_id |= *buffer++;
	}

	world->width = *buffer++;
	world->height = *buffer++;

	world->num_waypoints = *buffer++;

	for (i = 0; i < world->num_waypoints; i++, buffer += sizeof(point_t)) {
		memcpy(&world->waypoints[i], buffer, sizeof(point_t));
	}

	world->num_walls = *buffer++;
	for (i = 0; i < world->num_walls; i++, buffer += sizeof(wall_t)) {
		memcpy(&world->walls[i], buffer, sizeof(wall_t));
	}
}
/*
 * Method to send solution to a world
 */
void verify_solution(world_t* world, u32 path_len) {
	//Clear any received messages
	XEmacLite_FlushReceive(&ether);

	int i;
	u8 *buffer = tmit_buffer;

	//Write the destination MAC address (broadcast in this case)
	*buffer++ = 0x00;
	*buffer++ = 0x11;
	*buffer++ = 0x22;
	*buffer++ = 0x44;
	*buffer++ = 0x00;
	*buffer++ = 0x50;

	// Write the source MAC address
	for (i = 0; i < 6; i++)
		*buffer++ = mac_address[i];

	// Write the type
	*buffer++ = 0x55;
	*buffer++ = 0xAB;

	// Write message # and size
	*buffer++ = 0x03;
	*buffer++ = world->size;
	// Decompose world_id into 4 bytes
	*buffer++ = (world->id & 0xFF000000) >> 24;
	*buffer++ = (world->id & 0xFF0000) >> 16;
	*buffer++ = (world->id & 0xFF00) >> 8;
	*buffer++ = (world->id & 0xFF);

	// Single byte to enable walls
	*buffer++ = 0;

	// Decompose solution length into 4 bytes
	*buffer++ = (path_len & 0xFF000000) >> 24;
	*buffer++ = (path_len & 0xFF0000) >> 16;
	*buffer++ = (path_len & 0xFF00) >> 8;
	*buffer++ = (path_len & 0xFF);

	// Transmit
	XEmacLite_Send(&ether, tmit_buffer, 11 + XEL_HEADER_SIZE);
}

int receive_reply() {
	volatile int recv_len = 0;
	while (recv_len == 0) {
		recv_len = XEmacLite_Recv(&ether, recv_buffer);
		// Ensure transmission is not a cat fact
		if (memcmp2(recv_buffer, mac_address, 6) != 0) {
			recv_len = 0;
		}
	}

	return recv_buffer[15];
}
