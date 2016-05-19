#ifndef RIEMANN_H
#define RIEMANN_H

/* Information that is passed into pthread_create(). 
   Since multiple arguments are passed, a structure is created that contains
   all of the arguments, and then passed a pointer to that structure cast to
   (void *) */
   
struct thread_data
{
    short thread_id;                    /* Stores thread_id */
    double lbound;                      /* Stores global left bound */
    double rbound;                      /* Stores global right bound */
    
    double local_sum;                   /* Stores local sum */
    int curr_location;                  /* Tracks the thread's current
                                           working location */
    int parts;                          /* Total number of partitions the thread
                                           is working in */
    int remaining_parts;                /* Number of partitions remaining */
};

#endif /* riemann.h */