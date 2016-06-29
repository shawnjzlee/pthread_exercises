#ifndef RBARRIER_H
#define RBARRIER_H

#include <iostream>
#include <pthread.h>

using namespace std;

class rbarrier
{
    public:
        rbarrier(); // default constructor'
        rbarrier(int num_threads);
        ~rbarrier(); // destructor

        template <typename b_fn, typename v_fn>
        bool r_barrier_wait(pthread_barrier_t &barr,
                            const b_fn& condition,
                            const v_fn& callback);
                            
    private:
        pthread_barrier_t barrier;
        
};

rbarrier::rbarrier() {
    // pthread_barrier_init needs a count > 0, how to init?
}

rbarrier::rbarrier(int num_threads) {
    cout << "Constructed rbarrier class" << endl;
    pthread_barrier_init(&barrier, NULL, num_threads);
}

rbarrier::~rbarrier() {
    pthread_barrier_destroy(&barrier);
}

template <typename b_fn, typename v_fn>
bool rbarrier::r_barrier_wait (pthread_barrier_t &barr, 
                               const b_fn& condition,
                               const v_fn& callback) {
                         
    const bool result = condition ();
    
    if (result) { callback (); }
    pthread_barrier_wait (&barr);
    
    return true;
}

#endif