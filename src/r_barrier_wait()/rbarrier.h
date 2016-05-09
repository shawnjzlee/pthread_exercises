#ifndef RBARRIER_H
#define RBARRIER_H

using namespace std;

template <typename b_fn, typename v_fn>
bool r_barrier_wait (pthread_barrier_t * barrier, const b_fn& condition,
                     const v_fn& callback) {
    bool result = condition ();
    if (result) { callback (); }
    else return result;
}

#endif