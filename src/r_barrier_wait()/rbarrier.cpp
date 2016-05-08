#include <iostream>
#include <pthread.h>
using namespace std;

template <typename b_fn, typename v_fn>
bool r_barrier_wait (pthread_barrier_t * barrier, const b_fn& condition,
                     const v_fn& callback) {
    bool result = condition ();
    if (result) { callback (); }
    else return result;
}

int main()
{
    auto cond_l = [](void) {
        cout << "Do work!" << endl;
        return true;
    };
    
    auto cb_l = [](void) {
        cout << "Do more work!" << endl;
    };
    
    pthread_barrier_t barrier_temp;
    pthread_barrier_init(&barrier_temp, NULL, 2);
    
    /* TODO: init more pthread, do other work */
    
    cout << r_barrier_wait(&barrier_temp, cond_l, cb_l);
    
    /* TODO: destroy pthread */

    pthread_barrier_destroy(&barrier_temp);
}