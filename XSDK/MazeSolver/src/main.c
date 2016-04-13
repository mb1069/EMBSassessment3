#include "platform.h"
#include "xparameters.h"
#include "xuartlite_l.h"
#include "graphics.h"
#include "ethernet.h"
#include "main.h"


int getInput();

// Prototypes

int main() {
	xil_printf("\n\r\n");
	init_graphics();
    init_ethernet();

    while (1){
    	xil_printf("Enter world id: ");
    	int world_id = getInput();

    	xil_printf("\n\rEnter world size: ");
    	int world_size = getInput();
    	xil_printf("\n\r");

		world_t world = {.id = world_id, .size = world_size};

		xil_printf("Sending... \n\r");
		request_world(world_size, world_id); //Send a test frame
		xil_printf("Receiving... \n\r");
		receive_world(&world);

		xil_printf("Received world: \n\r");
		xil_printf("    - ID: %d \n\r", world.id);
		xil_printf("    - Size: %d \n\r", world.size);
		xil_printf("    - Height: %d \n\r", world.height);
		xil_printf("    - Width: %d \n\r", world.width);
		xil_printf("    - Num Waypoints: %d \n\r", world.num_waypoints);
		xil_printf("    - Num Walls: %d \n\r", world.num_walls);
		draw(&world);

		xil_printf("\n\r");
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
	return in;
}

