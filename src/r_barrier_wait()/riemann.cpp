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
#include <cstdio>
#include <ctime>

#include "rbarrier.h"
#include "riemann.h"
#include <pthread.h>

using namespace std;

double total;
double l_bound;
double r_bound;
double width;
int num_threads;

clock_t start;
double duration;

thread_data * thread_data_array;
rbarrier rbarrier;

double func (double x){ return x*x; }

void 
barrier_rc (int rc) {
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        cout << "Could not wait on barrier\n";
        exit(-1);
    }
}

/* Returns a double that calculates the total calculated from each thread.
   Work is done by main() */
void 
get_total (struct thread_data * thread_data_array) {
    double sum = 0;
    for(int i = 0; i < num_threads; i++) {
        sum += thread_data_array[i].local_sum;
    }
    cout << "The integral is: " << sum;
}

double 
thread_get_width (struct thread_data * data) {
    return data->rbound - data->lbound;
}

/* Passes in a thread argument from pthread_create(). This is the do_work 
   function that each thread works in. 
   Furthermore, lambda functions that allows r_barrier_wait() to check 
   condition and do_work is defined and called here. */
void * 
get_total (void * threadarg) {
    short tid = 0;
    thread_data * data;
    data = (thread_data *) threadarg;
    tid = data->thread_id;

    if (tid == 0) start = clock();
    // printf("#%hi is %f wide, with lbound at %f and rbound at %f.\n"
    //       , tid, thread_get_width(data), data->lbound, data->rbound);
    
    /* In each thread, calculate the area of each part given the function on
      line 29. */
    data->do_work();
    
    // printf("#%hi has a computed a sum of %f.\n", tid, data->local_sum);

    rbarrier.rbarrier_wait(
        [data](void)->bool {
            return data->get_sharing_condition(thread_data_array);
        } , 
        [data](void) {
            data->callback(thread_data_array);
        } );
    

}

int 
main(int argc, char * argv[])
{
   ifstream instream;
   
   int rc = 0, part_sz, l_bound, r_bound,
       remaining_parts = 0, i = 0, j = 0, index = 0;
    
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
    
    instream >> l_bound >> r_bound >> part_sz;
    
    width = (r_bound - l_bound) / (double)part_sz;
    
    if (num_threads > part_sz)
        num_threads = part_sz;
    
    rc = rbarrier.rbarrier_init(num_threads);
    barrier_rc(rc);
    
    pthread_t threads[num_threads];
    thread_data_array = (thread_data *)malloc(num_threads * sizeof(thread_data));
    
    for(int i = 0; i < num_threads; i++) {
        thread_data_array[i].thread_data_init(num_threads);
    }
    
    // pthread_barrier_init (&barrier, NULL, num_threads);
    // pthread_barrier_init (&_barrier, NULL, num_threads);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    if (part_sz  % num_threads)
        remaining_parts = part_sz  % (num_threads);

    double normal_dist = part_sz / num_threads;
    double ext_dist = normal_dist + 1;
    int num_norm_parts = (part_sz - (part_sz % num_threads));
    int num_ext_parts = part_sz - num_norm_parts;
    
    for (i = 0; i < num_norm_parts, index < num_threads - remaining_parts; index++)
    {
        if(num_ext_parts > 0 && i < num_ext_parts) {
            thread_data_array[index].thread_id = index;
            thread_data_array[index].lbound = l_bound + (width * ext_dist * index);
            thread_data_array[index].rbound = l_bound + (width * ext_dist * (index + 1));
            thread_data_array[index].curr_location = 0;
            thread_data_array[index].parts = ext_dist;
            thread_data_array[index].remaining_parts = ext_dist;
            thread_data_array[index].cond = 0;
            thread_data_array[index].width = width;
            i += ext_dist;
            rc = pthread_create(&threads[index], NULL, 
                                get_total, (void *) &thread_data_array[index]);
            if(rc)
            {
                printf("Return code from pthread_create() is %d \n", rc);
                exit(-1);
            }
        }
        else {
            thread_data_array[index].thread_id = index;
            thread_data_array[index].lbound = l_bound + (width * normal_dist * index);
            thread_data_array[index].rbound = l_bound + (width * normal_dist * (index + 1));
            thread_data_array[index].curr_location = 0;
            thread_data_array[index].parts = normal_dist;
            thread_data_array[index].remaining_parts = normal_dist;
            thread_data_array[index].cond = 0;
            thread_data_array[index].width = width;
            i += normal_dist;
            rc = pthread_create(&threads[index], NULL, 
                                get_total, (void *) &thread_data_array[index]);
            if(rc)
            {
                printf("Return code from pthread_create() is %d \n", rc);
                exit(-1);
            }
        }
    }


    void * status = 0;
    for(int t = 0; t < num_threads - 1; t++) {
        rc = pthread_join(threads[t], &status);
        if (rc) {
            printf("Error: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
    }

    duration = (clock() - start) / (double) CLOCKS_PER_SEC;
    cout << "Work Time: " << duration << endl;
    
    get_total(thread_data_array);
    
    pthread_attr_destroy(&attr);
    free (thread_data_array);
    pthread_exit(NULL);
}
