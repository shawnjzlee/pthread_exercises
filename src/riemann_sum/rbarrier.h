#ifndef RBARRIER_H
#define RBARRIER_H

#include <iostream>
#include <pthread.h>

using namespace std;

class rbarrier
{
    public:
        rbarrier();
        rbarrier(int num_threads);
        ~rbarrier();

        template <typename b_fn, typename v_fn>
        bool rbarrier_wait(const b_fn& condition,
                           const v_fn& callback);
                            
        int rbarrier_init(int num_threads);
                            
    private:
        pthread_barrier_t barrier;
};

rbarrier::rbarrier() { }

rbarrier::rbarrier(int num_threads) {
    pthread_barrier_init(&barrier, NULL, num_threads);
}

int rbarrier::rbarrier_init(int num_threads) {
    int rc = pthread_barrier_init(&barrier, NULL, num_threads);
    return rc;
}

rbarrier::~rbarrier() {
    pthread_barrier_destroy(&barrier);
}

template <typename b_fn, typename v_fn>
bool rbarrier::rbarrier_wait (const b_fn& condition,
                              const v_fn& callback) {
                         
    const bool result = condition ();
    
    if (result) { callback (); }
    pthread_barrier_wait (&barrier);
    
    return true;
}

#endif