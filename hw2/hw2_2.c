#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void local_sort(int* arrayp, int n);
void copy_list(int* local_list, int n, int* sort_list);
int cmp_int (const void * a, const void * b);

int main(){
	int id, comm_sz;
	int n, local_n, id0_temp_n, sort_list_len;
	int *list, *local_list, *sort_temp_list;
	int i; // for loop
    int phase;
    
    /*debug*/
    int *sol_list;
    
	srand((unsigned)time(NULL)); // for random

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	if (id == 0){
		printf("Enter N = : ");
		scanf("%d", &n);
		
		list = malloc(n*sizeof(int));
        /*debug*/
        sol_list = malloc(n*sizeof(int)); 
		for (i=0; i<n; i++){
			list[i] = rand();
            // debug
            sol_list[i] = list[i];
            printf("ORIGINAL LIST[%d] = %d\n", i, list[i]);
		}
	}
    
    local_n = n/comm_sz;
	MPI_Bcast(&local_n, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);

    MPI_Datatype newtype;
    MPI_Type_contiguous(local_n, MPI_INTEGER, &newtype);
    MPI_Type_commit(&newtype);
    
    if ( id == 0 ){
        for (i=1; i<comm_sz; i++){
            MPI_Send(list+(i-1)*local_n, 1, newtype, i, 0, MPI_COMM_WORLD);
        }
        id0_temp_n = local_n;
        local_n = n-local_n*(comm_sz-1);
        local_list = list+n-local_n;
    } else {
        local_list = malloc(local_n*sizeof(int));
        MPI_Recv(local_list, 1, newtype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    
    // first do local sort
    local_sort(local_list, local_n);
    
    // parallel odd-even sort
    if (id == 0){
        sort_temp_list = malloc( (local_n+id0_temp_n)*sizeof(int) );
        sort_list_len = (local_n+id0_temp_n);
    } else if (id < comm_sz-1){
        sort_temp_list = malloc( local_n*2*sizeof(int) );
        sort_list_len = local_n*2;
    }
    for ( phase=0; phase<n; phase++ ){
        if ( phase%2 == 0){
            // even phase
            if ( id%2 == 1 ){
                MPI_Send(local_list, 1, newtype, id-1, phase, MPI_COMM_WORLD);
                //printf("phase=%d id=%d send to id=%d\n", phase, id, id-1);
                MPI_Recv(local_list, 1, newtype, id-1, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("phase=%d id=%d receive from id=%d\n", phase, id, id-1);
            } else if ( id+1 < comm_sz ){
                MPI_Recv(sort_temp_list+local_n, 1, newtype, id+1, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("phase=%d id=%d receive from id=%d\n", phase, id, id+1);
                copy_list(local_list, local_n, sort_temp_list);
                local_sort(sort_temp_list, sort_list_len);
                copy_list(sort_temp_list, local_n, local_list);
                MPI_Send(sort_temp_list+local_n, 1, newtype, id+1, phase, MPI_COMM_WORLD);
                //printf("phase=%d id=%d send to id=%d\n", phase, id, id+1);
            }
            /*debug
            printf("\n\n AT phase %d :\n", phase);
            for (i=0; i<local_n; i++){
                printf("id=%d ; local_n=%d ; local_list[%d]=%d\n", id, local_n, i, local_list[i]);
            }
            */
        } else {
            // odd phase
            if ( id%2 == 0 && id != 0 ){
                MPI_Send(local_list, 1, newtype, id-1, phase, MPI_COMM_WORLD);
                //printf("phase=%d id=%d send to id=%d\n", phase, id, id-1);
                MPI_Recv(local_list, 1, newtype, id-1, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("phase=%d id=%d receive from id=%d\n", phase, id, id-1);
            } else if ( id != 0 && id+1 < comm_sz ) {
                MPI_Recv(sort_temp_list+local_n, 1, newtype, id+1, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //printf("phase=%d id=%d receive from id=%d\n", phase, id, id+1);
                copy_list(local_list, local_n, sort_temp_list);
                local_sort(sort_temp_list, sort_list_len);
                copy_list(sort_temp_list, local_n, local_list);
                MPI_Send(sort_temp_list+local_n, 1, newtype, id+1, phase, MPI_COMM_WORLD);
                //printf("phase=%d id=%d send to id=%d\n", phase, id, id+1);
            }
            /*debug
            printf("\n\n AT phase %d :\n", phase);
            for (i=0; i<local_n; i++){
                printf("id=%d ; local_n=%d ; local_list[%d]=%d\n", id, local_n, i, local_list[i]);
            }
            */
        }
    }
    
    /*debug*/
	for (i=0; i<local_n; i++){
		printf("ANS :: id=%d ; local_n=%d ; local_list[%d]=%d\n", id, local_n, i, local_list[i]);
	}
    /*debug*/
    if (id == 0){
        qsort(sol_list, n, sizeof(n), cmp_int);
        for (i=0; i<n; i++){
            printf("SOL :: list[%d]=%d\n", i, sol_list[i]);
        }
    }

    MPI_Type_free(&newtype);
	MPI_Finalize();
    
    if (id == 0)
        free(list);
    else
        free(local_list);
    free(sort_temp_list);
    
	return 0;
}

// use qsort for local sorting
void local_sort(int* arrayp, int n)
{
    qsort(arrayp, n, sizeof(int), cmp_int);
}

void copy_list(int* local_list, int n, int* sort_list)
{
    int i;
    
    for (i=0; i<n; i++){
        sort_list[i] = local_list[i];
    }
}

// for qsort
int cmp_int (const void * a, const void * b)
{
   return ( *(int*)a - *(int*)b );
}
