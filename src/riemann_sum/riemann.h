#ifndef RIEMANN_H
#define RIEMANN_H

#include <unistd.h>
#include "rbarrier.h"

class thread_data {
    public:
        thread_data();
        ~thread_data();
        
        void thread_data_init(int _num_threads);
        
        double func(double value);
        bool get_sharing_condition(thread_data * thread_data_array);
        void get_total(thread_data * thread_data_array);
        void callback(thread_data * thread_data_array);
        void do_work();
        
        pthread_mutex_t do_work_mutex;
    
        short thread_id;                    /* Stores thread_id */
        int num_threads;                    /* Stores the number of threads */
        double lbound;                      /* Stores global left bound */
        double rbound;                      /* Stores global right bound */
        
        int stolen_index;
        int stolen_location;
        int stolen_parts;
        
        double width;
        
        double local_sum;                   /* Stores local sum */
        int curr_location;                  /* Tracks the thread's current
                                               working location */
        int parts;                          /* Total number of partitions the thread
                                               is working in */
        int remaining_parts;                /* Number of partitions remaining */
        
        bool cond;                          /* Flags the condition of the thread */
    
};

thread_data::thread_data() { }

thread_data::~thread_data() {
    pthread_mutex_destroy(&do_work_mutex);
}

void thread_data::thread_data_init(int _num_threads) {
    num_threads = _num_threads;
    pthread_mutex_init(&do_work_mutex, NULL);
}

double thread_data::func(double value) {
    return value * value;
}

bool thread_data::get_sharing_condition(thread_data * thread_data_array) {
    for (stolen_index = 0; stolen_index < num_threads; stolen_index++)
    {
        if(stolen_index == thread_id) { continue; }
        pthread_mutex_lock(&thread_data_array[stolen_index].do_work_mutex);
        if((thread_data_array[stolen_index].curr_location < 
           (thread_data_array[stolen_index].parts / 2)) 
           && thread_data_array[stolen_index].cond != 1)
        {
            thread_data_array[stolen_index].cond = 1;
            stolen_parts = thread_data_array[stolen_index].parts;
            thread_data_array[stolen_index].parts /= 2;
            stolen_location = thread_data_array[stolen_index].parts;
            pthread_mutex_unlock(&thread_data_array[stolen_index].do_work_mutex);
            return true;
        }
        pthread_mutex_unlock(&thread_data_array[stolen_index].do_work_mutex);
    }
    return false;
}

void thread_data::callback(thread_data * thread_data_array) {
    double sum = 0.0;
    double local_lbound = thread_data_array[stolen_index].lbound + stolen_location * width; 
    while(stolen_location != stolen_parts) {
        pthread_mutex_lock(&thread_data_array[stolen_index].do_work_mutex);
        thread_data_array[stolen_index].remaining_parts--;
        sum += func(local_lbound) * width;
        thread_data_array[stolen_index].local_sum += func(local_lbound) * width;
        local_lbound += width;
        stolen_location += 1;
        pthread_mutex_unlock(&thread_data_array[stolen_index].do_work_mutex);
    }
}

void thread_data::do_work() {
    double sum = 0.0;
    double local_lbound = lbound; 
    for (int i = 0; i < parts; i++) {
        // if(thread_id == 0) usleep(1);
        pthread_mutex_lock(&do_work_mutex);
        remaining_parts--;
        local_sum += func(local_lbound) * width;
        sum += func(local_lbound) * width;
        local_lbound += width;
        curr_location = i;
        pthread_mutex_unlock(&do_work_mutex);
    }
}

#endif /* riemann.h */