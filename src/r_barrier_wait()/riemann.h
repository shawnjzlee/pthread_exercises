#ifndef RIEMANN_H
#define RIEMANN_H

#include "rbarrier.h"

class thread_data {
    public:
        thread_data();
        ~thread_data();
        double func(double value);
        bool get_sharing_condition();
        
        void get_total(thread_data * thread_data_array);
        void call_back();
        void do_work(thread_data * data);
    
        short thread_id;                    /* Stores thread_id */
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

thread_data::~thread_data() { }

double thread_data::func(double value) {
    return value * value;
}

bool thread_data::get_sharing_condition() { }

void thread_data::call_back() { }

void thread_data::do_work(thread_data * data) {
    for (int i = 0; i < data->parts; i++) {
        data->remaining_parts--;
        data->local_sum += func(data->lbound) * width;
        data->lbound += width;
        data->curr_location += width;
    }
}

#endif /* riemann.h */