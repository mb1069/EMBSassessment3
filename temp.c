#include <stdio.h>

int its = 0;
int matrix[12][12] = {
        {0,18,10,9,11,24,21,9,19,25, 0, 0},
        {18,0,28,19,19,36,3,15,1,37, 0, 0},
        {10,28,0,11,13,24,31,13,29,27, 0, 0 },
        {9,19,11,0,2,33,22,4,20,34, 0, 0 },
        {11,19,13,2,0,35,22,4,20,36, 0, 0 },
        {24,36,24,33,35,0,35,33,35,3, 0, 0},
        {21, 3,31,22,22,35,0,18,2,36, 0, 0 },
        {9, 15, 13, 4, 4, 33, 18, 0, 16, 34, 0, 0 },
        {19, 1, 29, 20, 20, 35, 2, 16, 0, 36, 0, 0 },
        {25, 37, 27, 34, 36, 3, 36, 34, 36, 0, 0, 0}, 
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};


int smallest_distance;

int path[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

int size = 10;

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
	while (i < size-1){
		p[i]--;
		int j = i%2==1 ? p[i] : 0;
		swap(tour+j+1, tour+i+1);
                //printf("Path (%d,%d,%d,%d) %d \n\r", tour[0], tour[1], tour[2], tour[3], its);       
                if (its%1000000==0){
                        printf("Its: %d\n\r", its);
                }
                its++;
		if (tour[0]==0){
		        int path_val = get_path_cost(matrix, tour, size);
			if (path_val<*smallest_distance){
                                
				*smallest_distance = path_val;
                                printf("Found new best path: ");
				for (i = 0; i < size; i++){
					best_tour[i] = tour[i];
                                        printf("%d ", tour[i]);
				}
                                printf("\n\r");
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
	//printf("Evaluating path %d %d %d %d\n\r", tour[0], tour[1], tour[2], tour[3]);
	int total_cost = 0;
	int i = 0;
	for (i = 0; i < size; i++){
		if (i<size-1){
			//printf("%d -> %d : %d \n\r", tour[i], tour[i+1], total_cost);
			total_cost += matrix[tour[i]][tour[i+1]];
		} else {
			//printf("%d -> %d : %d \n\r", tour[i], tour[0], total_cost);
			total_cost += matrix[tour[i]][tour[0]];
		}
	}
	//printf("Path value %d %d %d %d: %d\n\n\r", tour[0], tour[1], tour[2], tour[3], total_cost);
	return total_cost;
}

void main(){
	printf("Shortest path: %d\n\r", get_shortest_path(matrix, size, &smallest_distance, path));
printf("its: %d \n\r", its);
}
