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

//#include "rbarrier.h"
#include "riemann.h"
#include <pthread.h>

using namespace std;

double total;
double l_bound;
double r_bound;
int index;
double width;
int num_threads;

struct thread_data * thread_data_array;
pthread_barrier_t barrier;

#define STRINGIFY(Y) #Y
#define OUTPUT(X) cout << STRINGIFY(X) << ": " << X << endl;

double func (double x){ return x*x; }

template <typename b_fn, typename v_fn>
bool r_barrier_wait (pthread_barrier_t barr, const b_fn& condition,
                     const v_fn& callback) {
    bool result = condition ();
    pthread_barrier_wait (&barr);
    if (result) { callback (); }
    else return result;
    pthread_barrier_wait (&barr);
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

void 
barrier_rc (int rc) {
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        cout << "Could not wait on barrier\n";
        exit(-1);
    }
}

/* Passes in a thread argument from pthread_create(). This is the do_work 
   function that each thread works in. 
   Furthermore, lambda functions that allows r_barrier_wait() to check 
   condition and do_work is defined and called here. */
void * 
get_total (void * threadarg) {
    int rc = 0;
    short tid = 0;
    struct thread_data * data;
    data = (struct thread_data *) threadarg;
    tid = data->thread_id;
    
    // printf("#%hi is %f wide, with lbound at %f and rbound at %f.\n"
    //       , tid, thread_get_width(data), data->lbound, data->rbound);
    
    /* In each thread, calculate the area of each part given the function on
      line 29. */
    for (int i = 0; i < data->parts; i++) {
        data->remaining_parts--;
        data->local_sum += func(data->lbound) * width;
        data->lbound += width;
    }
    
    printf("#%hi has a computed a sum of %f.\n", tid, data->local_sum);
    // rc = r_barrier_wait(barrier,
    //     [](void) {
    //         for (index = 0; index < num_threads; index++)
    //         {
    //             if(thread_data_array[index].curr_location < (thread_data_array[index].parts / 2))
    //                 return true;
    //         }
    //         return false;
    //     } , 
    //     [](void) {
    //         for(index = (thread_data_array[index].parts / 2); index < thread_data_array[index].parts; index++){
    //             thread_data_array[index].local_sum += func(l_bound) * width;
    //             l_bound += width;
    //         }
    //     } );
        
    // barrier_rc (rc);
    // rc = pthread_barrier_wait (&barrier);
    // barrier_rc (rc);
}

int 
main(int argc, char * argv[])
{
   ifstream instream;
   
   int rc = 0, part_sz, l_bound, r_bound,
       remaining_parts = 0, i = 0, j = 0;
    
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
    
    pthread_t threads[num_threads];
    thread_data_array = (struct thread_data *)malloc(num_threads * sizeof(thread_data));
    
    pthread_barrier_init (&barrier, NULL, num_threads);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    if (part_sz  % num_threads)
        remaining_parts = part_sz  % (num_threads);

    int normal_dist = part_sz / num_threads;
    int ext_dist = normal_dist + 1;
    int num_norm_parts = (part_sz - (part_sz % num_threads));
    int num_ext_parts = part_sz - num_norm_parts;
    
    OUTPUT(l_bound); OUTPUT(r_bound); OUTPUT(normal_dist); OUTPUT(ext_dist);
    OUTPUT(num_norm_parts); OUTPUT(num_ext_parts); OUTPUT(width);
    OUTPUT (width * (double)normal_dist);
    for (i = 0; i < num_norm_parts, index < num_threads - remaining_parts; 
         i += normal_dist, index++)
    {
        thread_data_array[index].thread_id = index;
        thread_data_array[index].lbound = l_bound + (width * normal_dist * index);
        thread_data_array[index].rbound = l_bound + (width * normal_dist * (index + 1));
        thread_data_array[index].curr_location = 0;
        thread_data_array[index].parts = normal_dist;
        thread_data_array[index].remaining_parts = normal_dist;
        rc = pthread_create(&threads[index], &attr, 
                            get_total, (void *) &thread_data_array[index]);
        if(rc)
        {
            printf("Return code from pthread_create() is %d \n", rc);
            exit(-1);
        }
    }
    for (j = 0; j < num_ext_parts; i += ext_dist, j++, index++)
    {
        thread_data_array[index].thread_id = index;
        thread_data_array[index].lbound = l_bound + (width * ext_dist * index);
        thread_data_array[index].rbound = l_bound + (width * ext_dist * (index + 1));
        thread_data_array[index].curr_location = 0;
        thread_data_array[index].parts = ext_dist;
        thread_data_array[index].remaining_parts = ext_dist;
        rc = pthread_create(&threads[index], &attr, 
                            get_total, (void *) &thread_data_array[index]);
        if(rc)
        {
            printf("Return code from pthread_create() is %d \n", rc);
            exit(-1);
        }
    }
    
    //main thread computes total sum
    
    
    if (num_threads > 1)
    {
        void * status = 0;
        for(int t = 0; t < num_threads - 1; t++) {
            rc = pthread_join(threads[t], &status);
            if (rc) {
                printf("Error: return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
    }
    get_total(thread_data_array);
    // cout << "The integral is: " << total << endl;
    pthread_attr_destroy(&attr);
    pthread_barrier_destroy (&barrier);
    free (thread_data_array);
    pthread_exit(NULL);
}
