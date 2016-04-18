#include <stdio.h>

int matrix[12][12] = {
	//       A  B  C  D
	/**A**/ {0,  7, 3, 11, 0, 0, 0, 0, 0, 0, 0, 0},
	/**B**/ {7,  0, 8,  4, 0, 0, 0, 0, 0, 0, 0, 0},
	/**C**/ {3,  8, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0},
	/**D**/ {11, 4,12,  0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


int smallest_distance;

int path[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

int size = 4;

void swap(int *x, int *y){
	int temp;
	temp = *x;
	*x = *y;
	*y = temp;
}


void permute(int matrix[][12], int tour[12], int starting_index, int size, int* smallest_distance, int best_tour[12]){
	int p[13];
	int i;
	for (i = 0; i<size+1; i++){
		p[i]=i;
	}
	i = 0;
	while (i < size){
		p[i]--;
		int j = i%2==1 ? p[i] : 0;
		swap(tour+j, tour+i);

		if (tour[0]==0){
		        int path_val = get_path_cost(matrix, tour, size);
			if (path_val<*smallest_distance){
				*smallest_distance = path_val;
				for (i = 0; i < size; i++){
					best_tour[i] = tour[i];
				}
			}
		}
		i = 1;
		while (p[i]==0){
			p[i] = i;
			i++;
		}
	}
}

int get_shortest_path(int matrix[][12], int size, int* smallest_distance, int tour[12]){
	int initial_distance = get_path_cost(matrix, tour, size);
	int best_tour[12] = {1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0};
	permute(matrix, tour, 1, size, &initial_distance, best_tour);

	printf("\n\rBest path (%d,%d,%d,%d) \n\r\n\r", best_tour[0], best_tour[1], best_tour[2], best_tour[3]);
	return initial_distance;
}

int get_path_cost(int matrix[][12], int tour[12], int size){
	printf("Evaluating path %d %d %d %d\n\r", tour[0], tour[1], tour[2], tour[3]);
	int total_cost = 0;
	int i = 0;
	for (i = 0; i < size; i++){
		if (i<size-1){
			printf("%d -> %d : %d \n\r", tour[i], tour[i+1], total_cost);
			total_cost += matrix[tour[i]][tour[i+1]];
		} else {
			printf("%d -> %d : %d \n\r", tour[i], tour[0], total_cost);
			total_cost += matrix[tour[i]][tour[0]];
		}
	}
	printf("Path value %d %d %d %d: %d\n\n\r", tour[0], tour[1], tour[2], tour[3], total_cost);
	return total_cost;
}

void main(){
	printf("Shortest path: %d\n\r", get_shortest_path(matrix, size, &smallest_distance, path));
}
