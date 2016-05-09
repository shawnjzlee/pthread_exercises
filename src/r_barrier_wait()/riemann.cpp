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

// #include "rbarrier.h"
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

double func (double x){ return x*x; }

template <typename b_fn, typename v_fn>
bool r_barrier_wait (pthread_barrier_t * barrier, const b_fn& condition,
                     const v_fn& callback) {
    bool result = condition ();
    if (result) { callback (); }
    else return result;
}

auto cond_l = [](void) {
    for (index = 0; index < num_threads; index++)
    {
        if(thread_data_array[index].curr_location < thread_data_array[index].par / 2)
            return true;
    }
    return false;
};

auto cb_l = [](void) {
    for(index = (thread_data_array[index].par / 2); index < thread_data_array[index].par; index++){
        total += func(l_bound) * width;
        l_bound += width;
    }
};

void * get_total (void * threadarg)
{
    struct thread_data * data;
    data = (struct thread_data *) threadarg;
    
    for (int i = 0; i < data->par; i++) {
        data->curr_location = i;
        total += func(l_bound) * width;
        l_bound += width;
    }
}

int main(int argc, char * argv[])
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
    
    for (i = 0; i < num_norm_parts, index < num_threads - remaining_parts - 1; 
         i += normal_dist, index++)
    {
        thread_data_array[index].thread_id = index;
        thread_data_array[index].global_lbound = l_bound;
        thread_data_array[index].global_rbound = r_bound;
        thread_data_array[index].curr_location = 0;
        thread_data_array[index].par = normal_dist;
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
        thread_data_array[index].global_lbound = l_bound;
        thread_data_array[index].global_rbound = r_bound;
        thread_data_array[index].curr_location = 0;
        thread_data_array[index].par = ext_dist;
        rc = pthread_create(&threads[index], &attr, 
                            get_total, (void *) &thread_data_array[index]);
        if(rc)
        {
            printf("Return code from pthread_create() is %d \n", rc);
            exit(-1);
        }
    }
    
    r_barrier_wait(&barrier, cond_l, cb_l);
    rc = pthread_barrier_wait (&barrier);
    
    if (rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD)
    {
        cout << "Could not wait on barrier\n";
        exit(-1);
    }

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

    cout << "The integral is: " << total << endl;
    pthread_attr_destroy(&attr);
    pthread_barrier_destroy (&barrier);
    free (thread_data_array);
    pthread_exit(NULL);
}
