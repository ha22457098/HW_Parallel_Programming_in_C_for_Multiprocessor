#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int mylog2(int x);
int mypow2(int x);

int main(int argc, char *argv[]){
    int my_rank, comm_sz;
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    long long i, n;
    long long local_n;
    long long num_in_circle = 0;
    int j;
    if (my_rank == 0){
        printf("Enter the number of tosses : ");
        scanf("%lli", &n);
        for (j=1; j<comm_sz; j++){
            MPI_Send(&n, 1, MPI_LONG_LONG, j, 0, MPI_COMM_WORLD);
        }
        local_n = n-(n/(long long)(comm_sz))*(long long)(comm_sz-1);
    } else {
        MPI_Recv(&n, 1, MPI_LONG_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_n = n/(long long)comm_sz;
    }
    /*debug*/
    //printf("process %d 's local_n : %lli\n", my_rank, local_n);
    /*debug*/
    
    double startTime = 0.0;
    double totalTime = 0.0;
    startTime = MPI_Wtime();
    
    srand((unsigned)time(NULL));
    double x;
    double y;
    double distance;
    for(i=0; i<local_n; i++){
        x = ((double)rand()/RAND_MAX)*2.0-1.0;
        y = ((double)rand()/RAND_MAX)*2.0-1.0;
        distance = x*x + y*y;
        if( distance <= 1.0 )
            num_in_circle++;
    }
    
    // data transfer with Tree-structured
    int phase_need = mylog2(comm_sz);
    /*debug*/
    //printf("phase_need : %d\n", phase_need);
    /*debug*/
    
    int phase;
    long long temp;
    for( phase=1; phase<=phase_need; phase++ ){
        if( my_rank != 0 ){
            if( my_rank % mypow2(phase) ){
                MPI_Send(&num_in_circle, 1, MPI_LONG_LONG, my_rank-mypow2(phase-1), 0, MPI_COMM_WORLD);
                /*
                printf("process %d send to process %d\n", my_rank, my_rank-mypow2(phase-1));
                fflush(stdout);
                */
                break;
            } else {
                if( my_rank+mypow2(phase-1) < comm_sz){
                    MPI_Recv(&temp, 1, MPI_LONG_LONG, my_rank+mypow2(phase-1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    /*
                    printf("process %d receive from process %d\n", my_rank, my_rank+mypow2(phase-1));
                    fflush(stdout);
                    */
                    num_in_circle += temp;
                }
            }
        } else {
            MPI_Recv(&temp, 1, MPI_LONG_LONG, mypow2(phase-1), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            /*
            printf("process %d receive from process %d\n", my_rank, mypow2(phase-1));
            fflush(stdout);
            */
            num_in_circle += temp;
        }
    }
    
    totalTime = MPI_Wtime()-startTime;
    
    if( my_rank!=0 ){
        /*
        printf("process %d has finished.\n", my_rank);
        fflush(stdout);
        */
        ;
    } else {
        //printf("%lli / %lli\n", num_in_circle, n);
        printf("process %d has finished in time %f secs. PI = %.10f\n", my_rank, totalTime,(num_in_circle/(double)(n))*4.0);
        fflush(stdout);
    }
    
    MPI_Finalize();
    
    return 0;
}

int mylog2(int x){
    int a = 0;
    int c = 0;
    while( x!=1 ){
        if( x%2 ){
            x /= 2;
            a++;
            c = 1;
        } else {
            x /= 2;
            a++;
        }
    }
    return a+c;
}

int mypow2(int x){
    int i;
    int a=1;
    
    for( i=0; i<x; i++)
        a*=2;
    return a;
}