#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <vector>
#include <unistd.h>
#include <pthread.h>
#include "heat_distribution.h"

using namespace std;

/* Globally accessible variables, mutexes, and barriers */
double ** matrix;
double tolerance;
pthread_mutex_t mutex_col_update;
pthread_cond_t tolerance_threshold_cv;

int thread_get_columns (struct thread_data * data)
{
    return data->r_bound - data->l_bound + 1;
}

void output_matrix (double ** matrix, int row, int col)
{
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            cout << setprecision(4) << matrix[i][j] << " ";
        }
        cout << endl;
    }
    return;
}

void output_matrix_1d (vector <double> matrix, int row, int col)
{
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            cout << setprecision(4) << matrix[col * i + j] << " ";
        }
        cout << endl;
    }
    return;
}

/* TODO: Computation that updates each cell between the thread's l_bound and
   r_bound. Ensure that at the l_bound and r_bound, other threads are not
   accessing the same column(s). Use mutex_col_update to lock the left or
   right bounded column. */
   
/* Boundry cases:
    1) Top Left
    2) Bottom Left
    3) Top Right
    4) Bottom Right
    5) Column boundries (will vary depending on column row)
    */
void update_cell (struct thread_data * data)
{
    data->max_difference = 0;
    
    double initial, diff;
    for (int i = 1; i < data->r_bound; i++)
    {
        for (int j = 1; j < data->rows - 1; j++)
        {
            /* If loop is at the last column, stop calculations */
            if(i == data->columns - 1)
                break;
            
            initial = matrix[j][i];
            
            matrix[j][i] = ((matrix[j-1][i] + matrix[j][i-1] + matrix[j][i+1] +
                           matrix[j+1][i]) / 4.0);
            
            diff = matrix[j][i] - initial;
            
            if (diff > data->max_difference)
            {
                data->max_difference = diff;
            }
        }
    }
}

void * update_matrix(void * threadarg)
{
    short tid = 0;
    struct thread_data * data;
    
    data = (struct thread_data *) threadarg;
    tid = data->thread_id;
    
    printf("#%hi owns %i columns, with left at %i and right at %i.\n"
           , tid, thread_get_columns(data), data->l_bound, data->r_bound);
    
    do {
        update_cell (data);
    } while (data->max_difference > tolerance);
    
    output_matrix (matrix, data->rows, data->columns);
    
    pthread_exit(NULL);
}

/* This program passes in three arguments:
    - input file
    - output file
    - number of threads
   The program exits if the any of the arguments are missing */

int main(int argc, char * argv[])
{
    /* Create input and output streams to text files */
    ifstream instream;
    ofstream outstream;
    
    /* Main() thread attribute variables */
    int num_threads = 0, rc = 0, 
        cols_per_thread = 0, remaining_cols = 0,
        l_thread = 0, r_thread = 0;
    void * status;
    long tid;
    
    /* Interator variables */
    int i = 0, j = 0, k = 0, index = 0;
    
    /* Main() matrix attributes */
    double top, right, bottom, left;
    int row , column  = 0;

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
        cout << "Not enough arguments. \n"
             << "Requires [input file] [output file] [number of threads]. \n";
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

    /* Get all matrix related variables from input file */
    instream >> row  >> column 
             >> top >> right 
             >> bottom >> left 
             >> tolerance;

    /* Initialize the 2D array with the above initial values (H X W)*/
    vector <double> matrix_1d (row*column, 0);
    const int matrix_size = matrix_1d.size();
    
    for (i = 0; i < column; i++)
        matrix_1d[i] = top;
    for (i = column - 1; i < matrix_size; i += column)
        matrix_1d[i] = right;
    for (i = (row * column) - column; i < matrix_size; i++)
        matrix_1d[i] = bottom;
    for (i = 0; i <= matrix_size - column; i += column)
        matrix_1d[i] = left;
    
    matrix = (double **)malloc (row * sizeof (double *));
    for(i = 0; i < row ; i++)
        matrix[i] = (double *)malloc (column * sizeof (double));
        
    for (i = 0; i < column; i++)
        matrix[0][i] = top;
    for (i = 0; i < row; i++)
        matrix[i][column - 1] = right;
    for (i = 0; i < column; i++)
        matrix[row - 1][i] = bottom;
    for (i = 0; i < row; i++)
        matrix[i][0] = left;
    
    /* Create a vector of threads */
    vector <pthread_t> threads (num_threads);
        
    /* Initialize mutex and condition variable objects */
    pthread_mutex_init (&mutex_col_update, NULL);
    pthread_cond_init (&tolerance_threshold_cv, NULL);
    /* Initialize and set thread detached attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    if (num_threads == 0)
    {
        cout << "In main: creating thread " << index << endl;
        struct thread_data data;
        /* Parameters to include when creating threads.
           left and right are neighboring threads */
        if (index == 0) l_thread = 0;
        else l_thread = index - 1;
        if (index == (num_threads - 1)) r_thread = 0;
        else r_thread = index + 1;

        /* Populating the struct */
        data.thread_id = index;
        data.l_thread = l_thread;
        data.r_thread = r_thread;
        data.l_bound = 0;
        data.r_bound = column - 1;
        data.columns = column;
        data.rows = row;
        
        do {
            update_cell (&data);
        } while (data.max_difference > tolerance);
        output_matrix(matrix, row, column);
    }
    /* Partition the number of columns per thread. If there are too many
       requested threads, set it to the number of columns. Remaining number 
       of columns will be distributed evenly to the latter threads. */
    else
    {
        /* Declare the thread_data array after number of threads is known.
           This array will be passed as thread arguments. */
        struct thread_data thread_data_array[num_threads];   
        
        if (num_threads > column)
            num_threads = column;
        if (column  % num_threads)
            remaining_cols = column  % (num_threads);
        cols_per_thread = column / (num_threads);
        
        /* A column distribution algorithm that does not require MPI for threads
           to communicate. Reduces chance of threads to communicate 
           unnecessarily. */
           
        /* Sets the columns for normal distribution of columns */
        int normal_dist = column / num_threads;
        int ext_dist = normal_dist + 1;
        int num_norm_cols = (column - (column % num_threads));
        int num_ext_cols = column - num_norm_cols;
    
        for (i = 0; i < num_norm_cols, index < num_threads - remaining_cols; 
               i += normal_dist, index++)
        {
            cout << "In main: creating thread " << index << endl;
            
            /* Parameters to include when creating threads.
               left and right are neighboring threads */
            if (index == 0) l_thread = 0;
            else l_thread = index - 1;
            if (index == (num_threads - 1)) r_thread = 0;
            else r_thread = index + 1;
    
            /* Populating the struct */
            thread_data_array[index].thread_id = index;
            thread_data_array[index].l_thread = l_thread;
            thread_data_array[index].r_thread = r_thread;
            thread_data_array[index].l_bound = i;
            thread_data_array[index].r_bound = i + normal_dist - 1;
            thread_data_array[index].columns = column;
            thread_data_array[index].rows = row;
            rc = pthread_create(&threads[index], &attr, 
                                update_matrix, (void *) &thread_data_array[index]);
            if(rc)
            {
                printf("Return code from pthread_create() is %d \n", rc);
                exit(-1);
            }
        }
        /* Sets columns for extended distribution of columns 
           TODO: Reduce the number of iterators in the loop, if possible */
        for (j = 0; j < num_ext_cols; i += ext_dist, j++, index++)
        {
            cout << "In main: creating thread " << index << endl;
            
            /* Parameters to include when creating threads.
               left and right are neighboring threads */
            if (index == 0) l_thread = 0;
            else l_thread = index - 1;
            if (index == (num_threads - 1)) r_thread = 0;
            else r_thread = index + 1;
    
            /* Populating the struct */
            thread_data_array[index].thread_id = index;
            thread_data_array[index].l_thread = l_thread;
            thread_data_array[index].r_thread = r_thread;
            thread_data_array[index].l_bound = i;
            thread_data_array[index].r_bound = i + ext_dist - 1;
            thread_data_array[index].columns = column;
            thread_data_array[index].rows = row;
            rc = pthread_create(&threads[index], &attr, 
                                update_matrix, (void *) &thread_data_array[index]);
            if(rc)
            {
                printf("Return code from pthread_create() is %d \n", rc);
                exit(-1);
            }
        }
    }
    /* Free attribute and wait for the other threads */
    pthread_attr_destroy (&attr);
    while (!threads.empty ())
    {
        rc = pthread_join (threads[i], &status);
        if(rc)
        {
            printf("Return code from pthread_join() is %d \n", rc);
            
            for (i = 0; i < row; ++i)
                free (matrix[i]);
            free (matrix);
            
            exit(-1);
        }
        printf ("main(): program completed with thread %ld having a status of %ld \n", i, (long)status);
    }
    
    /* Last thing that main() should do */
    printf("main(): program completed. \n");
    // output_matrix (matrix, row, column);
    // for (i = 0; i < row; ++i)
    //     free (matrix[i]);
    // free (matrix);
    pthread_mutex_destroy (&mutex_col_update);
    pthread_cond_destroy (&tolerance_threshold_cv);
    pthread_exit(NULL);
}