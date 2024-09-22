#ifndef THREAD_FUNCTIONS_H
#define THREAD_FUNCTIONS_H

#include "structures.h"
#include "ridesharing.h"

void insertRequest (Producer* producer);
RequestType removeRequest (Consumer* consumer); //returns id of what was removed
void* consumer_function(void * VoidPtr);
void* producer_function(void * VoidPtr);

#endif