#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>


int main(){
	int id, comm_sz;
	int n, local_n;
	int *list, *local_list;
	int i; // for loop
	srand((unsigned)time(NULL));

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);

	if (id == 0){
		printf("Enter N = : ");
		scanf("%d", &n);
		
		list = malloc(n*sizeof(int));
		for (i=0; i<n; i++){
			list[i] = rand();
            printf("ORIGINAL LIST[%d] = %d\n", i, list[i]);
		}
	}
    
    local_n = n/comm_sz;
	MPI_Bcast(&local_n, 1, MPI_INTEGER, 0, MPI_COMM_WORLD);

    MPI_Datatype newtype;
    MPI_Type_contiguous( local_n, MPI_INTEGER, &newtype);
    MPI_Type_commit(&newtype);
    
    if ( id == 0 ){
        for (i=1; i<comm_sz; i++)
            MPI_Send(list+(i-1)*local_n, 1, newtype, i, 0, MPI_COMM_WORLD);
        local_n = n-local_n*(comm_sz-1);
        local_list = list+n-local_n;
    } else {
        local_list = malloc(local_n*sizeof(int));
        MPI_Recv(local_list, 1, newtype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
   
	/*debug*/
	for (i=0; i<local_n; i++){
		printf("id=%d ; local_n=%d ; local_list[%d]=%d\n", id, local_n, i, local_list[i]);
	}

    MPI_Type_free(&newtype);
	MPI_Finalize();
    
    if (id == 0)
        free(list);
    else
        free(local_list);
    
	return 0;
}
