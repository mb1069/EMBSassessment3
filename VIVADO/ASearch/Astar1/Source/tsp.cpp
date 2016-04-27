#include "toplevel.h"
int its = 0;

// Swap the ordering of two waypoints in path
void swap(u4 *x, u4 *y) {
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

// Generate all permutations for size waypoints, without generating mirror paths or paths not starting from waypoint 0
// Based on non-recursive quick_perm algorithm (see document for reference)
void permute(u16 matrix[][12], u4 tour[12], u4 size, u16* smallest_distance,
		u4 best_tour[12]) {
	u8 p[11];
	u8 i;
	permute_label14:for (i = 0; i<size - 1; i++){
		p[i]=0;
	}
    i = 1;
    permute_label12:while (i < size - 1){
    	if (p[i] < i){
    		u8 j = i%2 * p[i];
    		swap(tour+j+1, tour+i+1);
    		// Avoid mirrored paths
    		if (tour[1] < tour[size-1]){
				u16 path_val = get_path_cost(matrix, tour, size);

				if (path_val < *smallest_distance) {
					*smallest_distance = path_val;
					permute_label13:for (int x = 0; x < size; x++) {
						best_tour[x] = tour[x];
					}
				}
    		}
    		p[i]++;
    		i = 1;
    	} else {
    		p[i] = 0;
    		i++;
    	}
    }
}

// Initialise and evaluate non-mirror path
u16 get_shortest_loop(u16 matrix[][12], u4 size, u4 best_tour[12]) {
	u4 tour[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	u16 initial_distance = get_path_cost(matrix, tour, size);
	permute(matrix, tour, size, &initial_distance, best_tour);
	return initial_distance;
}

// Calculate cost of hamiltonian cycle through all waypoints starting at waypoint 0
u16 get_path_cost(u16 matrix[][12], u4 tour[12], u4 size) {
	u16 total_cost = 0;
	int i = 0;
	get_path_cost_label23:for (i = 0; i < size; i++) {
		if (i < size - 1) {
			total_cost += matrix[tour[i]][tour[i + 1]];
		} else {
			total_cost += matrix[tour[i]][tour[0]];
		}
	}
	return total_cost;
}
