#ifndef RIEMANN_H
#define RIEMANN_H

#include "rbarrier.h"

class thread_data {
    public:
        thread_data();
        ~thread_data();
        
        void thread_data_init(int _num_threads);
        
        double func(double value);
        bool get_sharing_condition(thread_data * data, int index);
        void get_total(thread_data * data);
        void callback(thread_data * data, int index);
        void do_work(thread_data * data);
    
        pthread_mutex_t get_sharing_mutex;
        pthread_mutex_t callback_mutex;
        pthread_mutex_t do_work_mutex;
    
        short thread_id;                    /* Stores thread_id */
        int num_threads;                    /* Stores the number of threads */
        double lbound;                      /* Stores global left bound */
        double rbound;                      /* Stores global right bound */
        
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
    pthread_mutex_destroy(&get_sharing_mutex);
    pthread_mutex_destroy(&callback_mutex);
    pthread_mutex_destroy(&callback_mutex);
}

void thread_data::thread_data_init(int _num_threads) {
    num_threads = _num_threads;
    pthread_mutex_init(&get_sharing_mutex, NULL);
    pthread_mutex_init(&callback_mutex, NULL);
    pthread_mutex_init(&callback_mutex, NULL);
}

double thread_data::func(double value) {
    return value * value;
}

bool thread_data::get_sharing_condition(thread_data * data, int index) {
    pthread_mutex_lock(&get_sharing_mutex);
    for (index = 0; index < num_threads; index++)
    {
        if((data[index].curr_location 
            < (data[index].parts / 2)) 
            || data[index].cond != 1)
        {
            data[index].cond = 1;
            data[index].parts /= 2;
            return true;
        }
    }
    return false;
    pthread_mutex_unlock(&get_sharing_mutex);
}

void thread_data::callback(thread_data * data, int index) {
    pthread_mutex_lock(&callback_mutex);
    data[index].curr_location = (data[index].parts / 2);
    while(data[index].curr_location != data[index].rbound) {
        data[index].remaining_parts--;
        data[index].local_sum += func(data[index].curr_location) * width;
        data[index].curr_location += width;
    }
    pthread_mutex_unlock(&callback_mutex);
}

void thread_data::do_work(thread_data * data) {
    pthread_mutex_lock(&do_work_mutex);
    for (int i = 0; i < data->parts; i++) {
        data->remaining_parts--;
        data->local_sum += func(data->lbound) * width;
        data->lbound += width;
        data->curr_location += width;
    }
    pthread_mutex_unlock(&do_work_mutex);
}

#endif /* riemann.h */