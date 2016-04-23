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
double * matrix;
double tolerance;
double max_difference;
pthread_mutex_t * mutex_array;
pthread_mutex_t mutex_difference;
pthread_barrier_t barrier_threshold;

int thread_get_columns (struct thread_data * data)
{
    return data->flr - data->ceiling + 1;
}

void output_array (double * matrix, int size)
{
    int i = 0;
    while (i != size)
    {
        cout << matrix[i] << " ";
        i++;
    }
    cout << endl;
    return;
}

void output_matrix (double * matrix, int row, int col)
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

/* TODO: Computation that updates each cell between the thread's ceiling and
   flr. Ensure that at the ceiling and flr, other threads are not
   accessing the same column(s). Use mutex_row_update to lock the left or
   right bounded column. */
void update_cell (struct thread_data * data)
{
    data->max_difference = 0;
    max_difference = 0;
    
    double initial, diff;
    
    for (int i = 1; i < data->flr; i++)
    {
        for (int j = 1; j < data->columns - 1; j++)
        {
            pthread_mutex_lock (&mutex_array[i-1]);
            pthread_mutex_lock (&mutex_array[i]);
            pthread_mutex_lock (&mutex_array[i+1]);
            
            initial = matrix[data->columns * (i) + (j)];

            matrix[data->columns * (i) + (j)] = 
                ((matrix[data->columns * (i - 1) + (j)] +
                  matrix[data->columns * (i) + (j - 1)] +
                  matrix[data->columns * (i) + (j + 1)] +
                  matrix[data->columns * (i + 1) + (j)]) / 4.0);
                                       
            diff = matrix[data->columns * (i) + (j)] - initial;
            
            if (diff > data->max_difference)
            {
                /*pthread_mutex_lock (&mutex_difference);*/
                data->max_difference = diff;
                /*pthread_mutex_unlock (&mutex_difference);*/
            }

            pthread_mutex_unlock (&mutex_array[i-1]);
            pthread_mutex_unlock (&mutex_array[i]);
            pthread_mutex_unlock (&mutex_array[i+1]);
        }
    }
    if (max_difference > data->max_difference)
    {
        pthread_mutex_lock (&mutex_difference);
        max_difference = data->max_difference;
        pthread_mutex_unlock (&mutex_difference);
    }
}

void * update_matrix(void * threadarg)
{
    // short tid = 0;
    struct thread_data * data;
    
    data = (struct thread_data *) threadarg;
    // tid = data->thread_id;
    
    // printf("#%hi owns %i rows, with top at %i and bottom at %i.\n"
    //       , tid, thread_get_columns(data), data->ceiling, data->flr);
           
    // printf("#%hi has a neighbor at %i and at %i.\n", tid, data->t_thread, data->b_thread);
    
    do {
        update_cell (data);
        int rc = pthread_barrier_wait (&barrier_threshold);
        if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
        {
            cout << "Could not wait on barrier\n";
            exit(-1);
        }
    } while (max_difference > tolerance);
}

int main(int argc, char * argv[])
{
    /* Create input and output streams to text files */
    ifstream instream;
    
    /* Main() thread attribute variables */
    int num_threads = 0, rc = 0, 
        rows_per_thread = 0, remaining_rows = 0,
        t_thread = 0, b_thread = 0;
    void * status;
    
    /* Interator variables */
    int i = 0, j = 0, index = 0;
    
    double top, right, bottom, left;
    int row , column;

    string input_file;
    
    if(argc != 3)
    {
        cout << "Not enough arguments. \n"
             << "Requires [input file] [number of threads]. \n";
        return -1;
    }

    input_file = argv[1];
    
    instream.open(input_file.c_str());
    if(!instream.is_open()){
        cout << "Could not open file " << input_file << endl;
        return -1;
    }

    if(atoi(argv[2]) == 0)
        num_threads = 1;
    else
        num_threads = atoi(argv[2]);

    /* Get all matrix related variables from input file */
    instream >> row >> column 
             >> top >> right 
             >> bottom >> left 
             >> tolerance;

    /* Initialize the 1D array with the above initial values (H X W)*/
    matrix = (double *)malloc (row * column * sizeof(double));
    const int matrix_size = row * column;
    
    for (i = 0; i < column; i++)
        matrix[i] = top;
    for (i = column - 1; i < matrix_size; i += column)
        matrix[i] = right;
    for (i = (row * column) - column; i < matrix_size; i++)
        matrix[i] = bottom;
    for (i = 0; i <= matrix_size - column; i += column)
        matrix[i] = left;

    if (num_threads > row)
    {
        num_threads = row;
    }
    pthread_t threads[num_threads];
    struct thread_data thread_data_array[num_threads];  
        
    mutex_array = (pthread_mutex_t *)malloc (row * sizeof(pthread_mutex_t));
    for (i = 0; i < row; i++)
    {
        pthread_mutex_init (&mutex_array[i], NULL);
    }
    pthread_mutex_init (&mutex_difference, NULL);
    
    pthread_barrier_init (&barrier_threshold, NULL, num_threads);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    if (num_threads > row)
        num_threads = row;
    if (row  % num_threads)
        remaining_rows = row  % (num_threads);
    rows_per_thread = row / (num_threads);

    int normal_dist = row / num_threads;
    int ext_dist = normal_dist + 1;
    int num_norm_rows = (row - (row % num_threads));
    int num_ext_rows = row - num_norm_rows;
    
    for (i = 0; i < num_norm_rows, index < num_threads - remaining_rows - 1; 
         i += normal_dist, index++)
    {
        
        if (index == 0) t_thread = -1;
        else t_thread = index - 1;
        if (index == (num_threads - 1)) b_thread = -1;
        else b_thread = index + 1;
        
        thread_data_array[index].thread_id = index;
        thread_data_array[index].t_thread = t_thread;
        thread_data_array[index].b_thread = b_thread;
        thread_data_array[index].ceiling = i;
        thread_data_array[index].flr = i + normal_dist - 1;
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
    for (j = 0; j < num_ext_rows; i += ext_dist, j++, index++)
    {
        if (index == 0) t_thread = -1;
        else t_thread = index - 1;
        if (index == (num_threads - 1)) b_thread = -1;
        else b_thread = index + 1;

        thread_data_array[index].thread_id = index;
        thread_data_array[index].t_thread = t_thread;
        thread_data_array[index].b_thread = b_thread;
        thread_data_array[index].ceiling = i;
        thread_data_array[index].flr = i + ext_dist - 1;
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
    
    if (index == 0) t_thread = -1;
    else t_thread = index - 1;
    if (index == (num_threads - 1)) b_thread = -1;
    else b_thread = index + 1;

    /* Populating the struct */
    thread_data_array[index].thread_id = index;
    thread_data_array[index].t_thread = t_thread;
    thread_data_array[index].b_thread = b_thread;
    thread_data_array[index].ceiling = i;
    thread_data_array[index].flr = i + normal_dist - 1;
    thread_data_array[index].columns = column;
    thread_data_array[index].rows = row;
    
    update_matrix(&thread_data_array[index]);
    
    if (num_threads > 1)
    {
        for(int t = 0; t < num_threads - 1; t++) {
            rc = pthread_join(threads[t], &status);
            if (rc) {
                printf("Error: return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
    }

    output_matrix(matrix, row, column);
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy (&mutex_difference);
    pthread_barrier_destroy (&barrier_threshold);
    free (mutex_array);
    free (matrix);
    pthread_exit(NULL);
}