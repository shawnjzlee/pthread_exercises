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
    short thread_id;                    /* Stores thread_id */
    short l_thread;                     /* Contains left neighboring thread */
    short r_thread;                     /* Contains right neighboring thread */
    int l_bound;                        /* Contains the left-most column */
    int r_bound;                        /* Contains the right-most column */
    
    /* Variables to check for correctness */
    int columns;                        /* Number of columns in matrix. */
    int rows;                           /* Number of rows in matrix. */
    double max_difference;              /* Max difference after updating the
                                           thread bounds */
};

/* Globally accessible variables and mutex */
double ** matrix;
double tolerance;
pthread_mutex_t mutex_col_update;
// struct thread_data * thread_data_array;

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
            cout << matrix[i][j] << " ";
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
    5) Column boundries (will vary depending on column row )
    */
void update_cell (struct thread_data * data)
{
    double initial = 0, diff = 0;
    /* Traverse top-bottom */
    for (int i = 0; i < data->rows; i++)
    {
        for (int j = 0; j < data->columns; j++)
        {
            initial = matrix[i][j];
            
            /* else if ceiling */
            if (i == 0)
            {
                if (j == 0)
                    matrix[i][j] = ((matrix[i+1][j] + matrix[i][j+1]) / 2 );
                else
                {
                    matrix[i][j] = ((matrix[i][j-1] + matrix[i+1][j] 
                                     + matrix[i][j+1]) / 3);
                }
            }
            /* else if left wall */
            else if (j == 0)                     
            {
                if (i == data->rows - 1)
                    matrix[i][j] = ((matrix[i-1][j] + matrix[i][j+1]) / 2 );
                else
                {
                    matrix[i][j] = ((matrix[i+1][j] + matrix[i-1][j] 
                                     + matrix[i][j+1]) / 3 );
                }
            }
            /* else if floor */
            else if (i == data->rows - 1)
            {
                if (j == data->columns - 1)
                    matrix[i][j] = ((matrix[i-1][j] + matrix[i][j-1]) / 2 );
                else
                {
                    matrix[i][j] = ((matrix[i][j-1] + matrix[i-1][j] 
                                     + matrix[i][j+1]) / 3);
                }
            }
            /* else if right wall */
            else if (j == data->columns - 1)         
            {
                if (i == 0)
                    matrix[i][j] = ((matrix[i+1][j] + matrix[i][j-1]) / 2 );
                else
                {
                    matrix[i][j] = ((matrix[i+1][j] + matrix[i][j-1] 
                                     + matrix[i-1][j]) / 3 );
                }
            }
            else
            {
                matrix[i][j] = ((matrix[i-1][j] + matrix [i][j-1] 
                                 + matrix [i][j+1] + matrix [i+1][j])/4);
            }
            diff = matrix[i][j] - initial;
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
    
    // do
    // {
        update_cell (data);
    // } while (data->max_difference > tolerance);
    
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
        
    long tid;
    
    /* Interator variables */
    int i = 0, j = 0, k = 0, index = 0;
    
    /* Miscellaneous variables */
    // bool divisible = true;
    
    /* Main() matrix attributes */
    double top_left, top_right, bottom_left, bottom_right;
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

    /* Declare the thread_data array after number of threads is known.
       This array will be passed as thread arguments. */
    struct thread_data thread_data_array[num_threads];
    
    /* Get all matrix related variables from input file */
    instream >> row  >> column 
             >> top_left >> top_right 
             >> bottom_left >> bottom_right 
             >> tolerance;

    /* Initialize the 2D array with the above initial values (H X W)*/
    matrix = new double * [row];
    for(i = 0; i < row ; ++i)
        matrix[i] = new double [column];
    matrix[0][0] = top_left;
    matrix[0][column - 1] = top_right;
    matrix[row - 1][0] = bottom_left;
    matrix[row - 1][column - 1] = bottom_right;
    
    /* Partition the number of columns per thread. If there are too many
       requested threads, set it to the number of columns. Remaining number 
       of columns will be distributed evenly to the latter threads. */
    if (num_threads > column)
        num_threads = column;
    if (column  % num_threads)
        remaining_cols = column  % (num_threads);
    cols_per_thread = column / (num_threads);
    
    // cout << "Columns per thread (Ext + 1): " << cols_per_thread << endl;
    // cout << "Remaining cols: " << remaining_cols << endl;
    // cout << "Number of threads: " << num_threads << endl;
    // cout << "Rows / Columns: " << column << " / " << column << endl << endl;

    /* Create the array of threads */
    pthread_t threads[num_threads];
    /* Initialize the mutex */
    pthread_mutex_init (&mutex_col_update, NULL);
    /* Initialize and set thread detached attribute */
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
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
        thread_data_array[index].l_thread = r_thread;
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
        thread_data_array[index].l_thread = r_thread;
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
    
    /* Last thing that main() should do */
    printf("main(): program completed. \n");
    output_matrix (matrix, row, column);
    for (i = 0; i < row; ++i)
        delete [] matrix[i];
    delete [] matrix;
    pthread_exit(NULL);
}