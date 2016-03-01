#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <pthread.h>

using namespace std;

struct thread_data
{
    long thread_id;                     /* Stores thread_id */
    int left;                           /* Contains left neighboring thread */
    int right;                          /* Contains right neighboring thread */
    double ** submatrix;                /* Dynamically created 2D array */
};

/* Global variable for thread data */
// struct thread_data * thread_data_array;

void * create_thread(void * threadarg)
{
    /* ... */
    
    pthread_exit(NULL);
}


int main(int argc, char * argv[])
{
    /* Create input and output streams to text files */
    ifstream instream;
    ofstream outstream;
    
    /* Main() thread attribute variables */
    int num_threads = 0, rc = 0, 
        cols_per_thread = 0, remaining_cols = 0,
        left = 0, right = 0;
        
    long tid;
    
    /* Interator variables */
    int i = 0, j = 0, k = 0;
    
    /* Main() matrix attributes */
    double top_left, top_right, bottom_left, bottom_right, tolerance = 0.0;
    int row, column = 0;

    /* Input file string */
    string input_file;
    /* Output file string */
    string output_file;
    
    /* Error checking: number of arguments passed into main().
           argv[1]: input_file name
           argv[2]: output_file name
           argv[3]: number of threads
       If number of arguments aren't met, return -1. */
    if(argc != 4)
    {
        cout << "Not enough arguments, exit(-1).";
        return -1;
    }

    input_file = argv[1];
    
    /* Error checking: input file can be opened */
    instream.open(input_file.c_str());
    if(!instream.is_open()){
        cout << "Could not open file " << input_file << endl;
        return -1;
    }

    output_file = argv[2];
    
    /* Error checking: output file can be opened */
    outstream.open(output_file.c_str());
    if(!outstream.is_open()){
        cout << "Could not open file " << output_file << endl;
        return -1;
    }

    num_threads = atoi(argv[3]);
    
    /* Declare the thread_data array after number of threads is known.
       This array will be passed as thread arguments. */
    struct thread_data thread_data_array[num_threads];
    
    /* Get all matrix related variables from input file */
    instream >> row >> column
             >> top_left >> top_right 
             >> bottom_left >> bottom_right 
             >> tolerance;
    
    /* Create the matrix with the received row and column variables */
    double matrix[row][column];
    
    /* Partition the number of columns per thread. Add an extra thread if there
       is a remainder after partitioning. */
    cols_per_thread = column / num_threads;
    remaining_cols = column % num_threads;
    if(remaining_cols)
    {
        num_threads+=1;
    }
    
    /* Create the array of threads */
    pthread_t threads[num_threads];
    
    /* Initialize threads with arguments passed into the threads */
    for(; i < num_threads; i++)
    {
        cout << "In main: creating thread" << i << endl;
        
        /* Parameters to include when creating threads.
           left and right are neighboring threads */
        if (i == 1) left = 0;
        else left = i - 1;
        if (i == num_threads) right = 0;
        else right = i + 1;
        
        /* Populating the struct */
        thread_data_array[i].thread_id = i;
        thread_data_array[i].left = left;
        thread_data_array[i].right = right;
        thread_data_array[i].submatrix = new double *[row];
        for(; j < row; j++)
            thread_data_array[i].submatrix[j] = new double[cols_per_thread];
        
        
        rc = pthread_create(&threads[i], NULL, 
                            create_thread, (void *) &thread_data_array[i]);
        if(rc)
        {
            printf("Return code from pthread_create() is %d \n", rc);
            exit(-1);
        }
    }
    
    
    
    /* Last thing that main() should do */
    pthread_exit(NULL);
}