#include "platform.h"
#include "xparameters.h"
#include "xuartlite_l.h"
#include "graphics.h"
#include "ethernet.h"
#include "main.h"
#include "fsl.h"

int getInput();

// Prototypes

int main() {
	int i;
	xil_printf("\n\r\n");
	init_graphics();
    init_ethernet();

    while (1){
    	xil_printf("Enter world id: ");
    	int world_id = getInput();

    	xil_printf("Enter world size: ");
    	int world_size = getInput();
		world_t world = {.id = world_id, .size = world_size};

		xil_printf("Sending... \n\r");
		request_world(world_size, world_id); //Send a test frame
		receive_world(&world);

		xil_printf("Received world: \n\r id: %d size: %d waypoints: %d walls: %d\n\r", world.id, world.size, world.height, world.num_waypoints, world.num_walls);
		draw(&world);
	    u32 testdata[27];
	    u32 info = 0;
	    if(world.size==0){
	    		info |= ((u32) 10) << 16;
	    } else if (world.size == 1){
	    		info |= ((u32) 30) << 16;
	    } else if (world.size == 2){
	    		info |= ((u32) 60) << 16;
	    }

	    info |= ((u32) world.num_waypoints) << 8;
	    info |= ((u32) world.num_walls);
	    testdata[0] = info;

	    memcpy(&testdata[1],&world.waypoints,  sizeof(world.waypoints[0])*world.num_waypoints);
	    memcpy(&testdata[1+(world.num_waypoints/2)], &world.walls, sizeof(world.walls[0])*world.num_walls);

//	     TODO check memcpy against vivado
//	     000a0406 02070002 05060302 01000800
//	     03010305 02010409 04000203 04010504 01010803
	    int data_i = 0;
//	    for (i = 0; i<world.num_waypoints; i= i + 2){
//	    	u32 *data = (u32*) &world.waypoints[i];
//	    	xil_printf("%08x ", (int) *data);
//	    	testdata[1+(data_i++)] = *data;
//	    }
//	    for (i = 0; i<world.num_walls; i++){
//	    	u32 *data = (u32*) &world.walls[i];
//	    	xil_printf("%08x ", (int) *data);
//	    	testdata[1+(data_i++)] = *data;
//	    }
	    for (i = 0; i<world.num_waypoints; i= i + 2){
	    	u32 data = 0;
	    	data |= (((u32)world.waypoints[i].x));
	    	data |= (((u32)world.waypoints[i].y)<< 8);
	    	data |= (((u32)world.waypoints[i+1].x) << 16);
	    	data |= (((u32)world.waypoints[i+1].y))  << 24;
//	    	xil_printf("%08x ", (int) data);
	    	testdata[1+(data_i++)] = data;
	    }
	    for (i = 0; i<world.num_walls; i++){
	    	u32 data = 0;
	    	data |= (((u32)world.walls[i].x));
	    	data |= (((u32)world.walls[i].y)<< 8);
	    	data |= (((u32)world.walls[i].dir) << 16);
	    	data |= (((u32)world.walls[i].len))  << 24;
//	    	xil_printf("%08x ", (int) data);
	    	testdata[1+(data_i++)] = data;
	    }

	    xil_printf("\n\r");
//	    for (i = 0; i<world.num_walls+(world.num_waypoints/2)+1; i++){
//	    	xil_printf("%08x ", (int) testdata[i]);
//	    }

	    for (i=0; i<1+data_i; i++){
	    	putfslx(testdata[i], 0, FSL_DEFAULT);
	    }
	    xil_printf("Sent to hardware \n\r");

	    u32 val;
	    getfslx(val, 0, FSL_DEFAULT);
	    i = 0;
	    while (val!='\n'){
	    	u8 x = (val >> 16) & 0xFF;
	    	u8 y = (val) & 0xFF;
	    	xil_printf("x: %d y: %d \n\r", (int) x, (int) y);
	    	drawPath(x, y);
	    	getfslx(val, 0, FSL_DEFAULT);
	    }
	    u32 shortest_path;

	    getfslx(shortest_path, 0, FSL_DEFAULT);
	    xil_printf("Got shortest path: %d\n\r", (int) shortest_path);



//
//
//	    getInput();
//	    solve_world(&world, shortest_path);
//	    xil_printf("Server replied %d \n\r", receive_reply());
//
//		xil_printf("Enter solution length: ");
//		u32 length = getInput();

//		int mark = receive_reply();
//		if (mark==0){
//			xil_printf("%d Correct solution!", length);
//		} else if (mark==1){
//			xil_printf("%d Solution was too long.", length);
//		} else if (mark==2){
//			xil_printf("%d Solution was too short.", length);
//		}
//		xil_printf("\n\r");

// Brute solver
//		int mark = 1;
//		u32 length = 0;
//		while (mark!=0){
//			solve_world(&world, length);
//			mark = receive_reply();
//			if (mark==0){
//				xil_printf("Correct solution: %d \n\r", (int) length);
//			}
//			length++;
//		}

    }
    return 0;
}

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

