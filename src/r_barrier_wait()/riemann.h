#ifndef RIEMANN_H
#define RIEMANN_H

/* Information that is passed into pthread_create(). 
   Since multiple arguments are passed, a structure is created that contains
   all of the arguments, and then passed a pointer to that structure cast to
   (void *) */
   
struct thread_data
{
    short thread_id;                    /* Stores thread_id */
    double global_lbound;               /* Stores global left bound */
    double global_rbound;               /* Stores global right bound */
    
    int curr_location;
    int par;
};

#endif /* riemann.h */