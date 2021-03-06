#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <cstdio>

#include "rbarrier.h"
#include "riemann.h"
#include <pthread.h>

using namespace std;
using namespace std::chrono;

#define STRINGIFY(Y) #Y
#define OUTPUT(X) cout << STRINGIFY(X) << ": " << X << endl;

double total;
double l_bound;
double r_bound;
double width;
int num_threads;

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
    double sum = 0.0;
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
    
    // printf("#%hi is %f wide, with lbound at %f and rbound at %f.\n"
    //       , tid, thread_get_width(data), data->lbound, data->rbound);
    
    high_resolution_clock::time_point start;
    if (tid == 0) 
        start = high_resolution_clock::now();
        
    data->do_work();
    
    rbarrier.rbarrier_wait(
        [data](void)->bool {
            return data->get_sharing_condition(thread_data_array);
        } , 
        [data](void) {
            data->callback(thread_data_array);
        } );

    if (tid == 0) {
        high_resolution_clock::time_point end = high_resolution_clock::now();
        duration<double> runtime = duration_cast<duration<double>>(end - start);
        cout << "Work Time (in secs): " << runtime.count() << endl;
    }
}

int 
main(int argc, char * argv[])
{
    ifstream instream;
   
    int rc = 0, part_sz, l_bound, r_bound,
       remaining_parts = 0, i = 0, j = 0, index = 0;
    
    double normal_dist = 0.0, init_dist = 0.0;
    
    string input_file;
    
    if(argc != 5)
    {
        cout << "Not enough arguments. \n"
             << "Requires [input file] [number of threads] [multiplier] [sharing flag]. \n";
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
    
    // multiplier increases work distribution to the first thread (#0)
    int multiplier = atoi(argv[3]);
    
    bool share_flag = atoi(argv[4]);
    
    instream >> l_bound >> r_bound >> part_sz;
    
    width = (r_bound - l_bound) / (double)part_sz;
    
    if (num_threads > part_sz)
        num_threads = part_sz;
    
    rc = rbarrier.rbarrier_init(num_threads);
    barrier_rc(rc);
    
    pthread_t threads[num_threads];
    thread_data_array = (thread_data *)malloc(num_threads * sizeof(thread_data));
    
    for(int i = 0; i < num_threads; i++) {
        thread_data_array[i].thread_data_init(num_threads, share_flag);
    }
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    if (part_sz  % (num_threads + multiplier))
        remaining_parts = part_sz  % (num_threads - 1);
 
    if (multiplier == 0 || num_threads == 1) normal_dist = part_sz / num_threads;
    else {
        init_dist = multiplier * (part_sz / (num_threads + multiplier));
        normal_dist = (part_sz - init_dist) / (num_threads - 1);
    }
    double ext_dist = normal_dist + 1;
    int num_norm_parts = part_sz - remaining_parts;
    int num_ext_parts = part_sz - num_norm_parts;
    
    if(num_threads == 1) {
        thread_data_array[index].thread_id = index;
        thread_data_array[index].lbound = l_bound + (width * normal_dist * index);
        thread_data_array[index].rbound = l_bound + (width * normal_dist * (index + 1));
        thread_data_array[index].curr_location = l_bound + (width * normal_dist * index);
        thread_data_array[index].parts = normal_dist;
        thread_data_array[index].cond = 0;
        thread_data_array[index].local_sum = 0;
        thread_data_array[index].width = width;
        
        high_resolution_clock::time_point start;
        start = high_resolution_clock::now();
        
        thread_data_array[index].do_work();
        
        high_resolution_clock::time_point end = high_resolution_clock::now();
        duration<double> runtime = duration_cast<duration<double>>(end - start);
        cout << "Work Time (in secs): " << runtime.count() << endl;
    }
    else {
        for (i = 0; index < num_threads - remaining_parts; index++)
        {
            thread_data_array[index].thread_id = index;
            if (multiplier && (i == 0)) {
                thread_data_array[index].lbound = l_bound + (width * init_dist * index);
                thread_data_array[index].rbound = l_bound + (width * init_dist * (index + 1));
                thread_data_array[index].curr_location = l_bound + (width * init_dist * index);
                thread_data_array[index].parts = init_dist;
                i += init_dist;
            }
            else if (multiplier) {
                thread_data_array[index].lbound = l_bound + (width * normal_dist * index) + (width * (init_dist - normal_dist));
                thread_data_array[index].rbound = l_bound + (width * normal_dist * (index + 1)) + (width * (init_dist - normal_dist));
                thread_data_array[index].curr_location = l_bound + (width * normal_dist * index) + (width * (init_dist - normal_dist));
                thread_data_array[index].parts = normal_dist;
                i += normal_dist;
            }
            else {
                thread_data_array[index].lbound = l_bound + (width * normal_dist * index);
                thread_data_array[index].rbound = l_bound + (width * normal_dist * (index + 1));
                thread_data_array[index].curr_location = l_bound + (width * normal_dist * index);
                thread_data_array[index].parts = normal_dist;
                i += normal_dist;
            }
            thread_data_array[index].cond = 0;
            thread_data_array[index].local_sum = 0;
            thread_data_array[index].width = width;
            rc = pthread_create(&threads[index], NULL, 
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
            thread_data_array[index].lbound = l_bound + (width * ext_dist * index) + (width * (init_dist - normal_dist));
            thread_data_array[index].rbound = l_bound + (width * ext_dist * (index + 1)) + (width * (init_dist - normal_dist));
            thread_data_array[index].curr_location = l_bound + (width * normal_dist * index) + (width * (init_dist - normal_dist));
            thread_data_array[index].parts = ext_dist;
            thread_data_array[index].cond = 0;
            thread_data_array[index].local_sum = 0;
            thread_data_array[index].width = width;
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

    get_total(thread_data_array);
    
    pthread_attr_destroy(&attr);
    free (thread_data_array);
    pthread_exit(NULL);
}
