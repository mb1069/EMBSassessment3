#include "platform.h"
#include "xparameters.h"
#include "xuartlite_l.h"
#include "graphics.h"
#include "ethernet.h"
#include "main.h"

#include "solver.h"

int getInput();

int main() {
	xil_printf("\n\r\n");
	// Initialise VGA and ethernet
	init_graphics();
	init_ethernet();

	while (1) {
		// Get world parameters
		xil_printf("Enter world id: ");
		int world_id = getInput();
		xil_printf("Enter world size: ");
		int world_size = getInput();

		// Store in structure
		world_t world = { .id = world_id, .size = world_size };

		// Request other parameters of the world given id and size
		xil_printf("	- Sending... \n\r");
		request_world(world_size, world_id);

		receive_world(&world);

		xil_printf("	- Received world: \n\r");
		xil_printf("	- id: %d size: %d waypoints: %d walls: %d\n\r",
				world.id, world.size, world.height, world.num_waypoints,
				world.num_walls);
		xil_printf("	- Drawing\n\r");
		draw(&world);

		u32 world_data[27];
		int data_i = pack(&world, world_data);
		solve(world_data, data_i);
		xil_printf("	- Sent to hardware \n\r");

		// Receive first 4 bytes of data corresponding to shortest found path through all waypoints
		u32 shortest_path = recv_u32();

		// Verify solution with server
		verify_solution(&world, shortest_path);

		// Receive path coordinates until 0xFFFF which signifies end of solution
		u32 res = recv_u32();

		while (res != (u32) 0xFFFF) {
			int x = (res >> 16) & 0xFF;
			int y = (res) & 0xFF;
			drawPath(x, y);
			res = recv_u32();
		}

		xil_printf("	- Got shortest path: %d\n\r", (int) shortest_path);

		// Receive reply from server
		int mark = receive_reply();
		xil_printf("	- Server replied %d \n\r", mark);
		switch (mark) {
		case 0:
			xil_printf("	- Correct solution!: %d \n\r", shortest_path);
			break;
		case 1:
			xil_printf("	- Solution was too long: %d \n\r", shortest_path);
			break;
		case 2:
			xil_printf("	- Solution was too short: %d \n\r", shortest_path);
			break;
		default:
			xil_printf("	- Invalid reply received. \n\r");
			break;
		}
		xil_printf("\n\r \n\r");
	}
	return 0;
}

/*
 * Method to retrieve world number/id from UART
 */
int getInput() {
	u32 in = 0;
	char b = '0';
	while ((b = XUartLite_RecvByte(XPAR_RS232_DTE_BASEADDR)) != '\r') {
		in = in * 10 + b - '0';
		XUartLite_SendByte(XPAR_RS232_DTE_BASEADDR, b);
	}
	xil_printf("\n\r");
	return in;
}

