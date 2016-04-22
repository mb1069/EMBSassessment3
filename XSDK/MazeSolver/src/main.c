#include "platform.h"
#include "xparameters.h"
#include "xuartlite_l.h"
#include "graphics.h"
#include "ethernet.h"
#include "main.h"
#include "fsl.h"

int getInput();


int main() {
	int i;
	xil_printf("\n\r\n");
	// Initialise VGA and ethernet
	init_graphics();
    init_ethernet();

    while (1){
    	// Get world parameters
    	xil_printf("Enter world id: ");
    	int world_id = getInput();

    	xil_printf("Enter world size: ");
    	int world_size = getInput();
    	// Store in structure
	world_t world = {.id = world_id, .size = world_size};

	xil_printf("Sending... \n\r");
	// Request other parameters of the world given id and size
	request_world(world_size, world_id);
	receive_world(&world);

	xil_printf("Received world: \n\r id: %d size: %d waypoints: %d walls: %d\n\r", world.id, world.size, world.height, world.num_waypoints, world.num_walls);

	draw(&world);

	// Data packing for communication to dedicated hardware
	u32 world_data[27];
	u32 info = 0;
	// Convert world size index into grid size
	if(world.size==0){
		info |= ((u32) 10) << 16;
	} else if (world.size == 1){
		info |= ((u32) 30) << 16;
	} else if (world.size == 2){
		info |= ((u32) 60) << 16;
	}
	// Pack rest of first byte with additional information
	info |= ((u32) world.num_waypoints) << 8;
	info |= ((u32) world.num_walls);
	world_data[0] = info;

	// Copy data of waypoints and then worlds into transmission array
	int data_i = 0;
	for (i = 0; i<world.num_waypoints; i= i + 2){
		u32 data = 0;
	    	data |= (((u32)world.waypoints[i].x));
	    	data |= (((u32)world.waypoints[i].y)<< 8);
	    	data |= (((u32)world.waypoints[i+1].x) << 16);
	    	data |= (((u32)world.waypoints[i+1].y))  << 24;
	    	world_data[1+(data_i++)] = data;
	}
	for (i = 0; i<world.num_walls; i++){
	    	u32 data = 0;
	    	data |= (((u32)world.walls[i].coords.x));
	    	data |= (((u32)world.walls[i].coords.y)<< 8);
	    	data |= (((u32)world.walls[i].dir) << 16);
	    	data |= (((u32)world.walls[i].len))  << 24;
	    	world_data[1+(data_i++)] = data;
	}

	// Send data to hardware
	for (i=0; i<1+data_i; i++){
	    	putfslx(world_data[i], 0, FSL_DEFAULT);
	}
	    xil_printf("Sent to hardware \n\r");


	// Receive first 4 bytes of data corresponding to shortest found path through all waypoints
	u32 shortest_path;
	getfslx(shortest_path, 0, FSL_DEFAULT);

	    // Verify solution with server
	solve_world(&world, shortest_path);

	// Receive path coordinates until 0xFFFF which signifies end of solution
	u32 res;
	getfslx(res, 0, FSL_DEFAULT);

	while (res != (u32) 0xFFFF){
		int x = (res >> 16) & 0xFF;
	    	int y = (res) & 0xFF;
	    	drawPath(x, y);
	    	getfslx(res, 0, FSL_DEFAULT);
	}
	xil_printf("\n\r");

	xil_printf("Got shortest path: %d\n\r", (int) shortest_path);

	// Receive reply from server
	int mark = receive_reply();
	xil_printf("Server replied %d \n\r", mark);
	switch (mark){
		case 0:
			xil_printf("Correct solution!: %d \n\r", shortest_path);
			break;
		case 1:
			xil_printf("Solution was too long: %d \n\r", shortest_path);
			break;
		case 2:
			xil_printf("Solution was too short: %d \n\r", shortest_path);
			break;
		default:
			xil_printf("Invalid reply received. \n\r");
			break;
	}
    }
    return 0;
}

/*
 * Method to retrieve world number/id from UART
 */
int getInput(){
	u32 in = 0;
	char b = '0';
	while((b= XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR)) != '\r') {
		in = in*10 + b - '0';
		XUartLite_SendByte(XPAR_RS232_DTE_BASEADDR, b);
	}
	xil_printf("\n\r");
	return in;
}

