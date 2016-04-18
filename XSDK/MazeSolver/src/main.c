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
		xil_printf("Receiving... \n\r");
		receive_world(&world);

		xil_printf("Received world: \n\r id: %d size: %d waypoints: %d walls: %d\n\r", world.id, world.size, world.height, world.num_waypoints, world.num_walls);
		draw(&world);

	    u32 testdata[27];
	    u32 info = 0;
//	    switch (world.size){
//	    	case 0:
//	    		info |= ((u32) 10) << 16;
//	    		break;
//	    	case 1:
//	    		info |= ((u32) 30) << 16;
//	    		break;
//	    	case 2:
//	    		info |= ((u32) 60) << 16;
//	    		break;
//	    }

	    info |= ((u32) world.num_waypoints) << 8;
	    info |= ((u32) world.num_walls);
	    testdata[0] = info;
	    xil_printf("%d \n\r", world.size);
	    xil_printf("%d \n\r", world.num_waypoints);
	    xil_printf("%d \n\r", world.num_walls);
	    xil_printf("%08x \n\r", (int) testdata[0]);
	    memcpy(&testdata[1],&world.waypoints,  sizeof(world.waypoints[0])*world.num_waypoints);
	    memcpy(&testdata[1+(world.num_waypoints/2)], &world.walls, sizeof(world.walls[0])*world.num_walls);
	    putfslx(testdata, 0, FSL_DEFAULT);


	    int i;
	    for (i = 0; i<world.num_walls+(world.num_waypoints/2)+1; i++){
	    	xil_printf("%08x ", (int) testdata[i]);
	    }
	    xil_printf("\r\n");
	    getInput();

//		xil_printf("Enter solution length: ");
//		u32 length = getInput();
//		solve_world(&world, length);
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

