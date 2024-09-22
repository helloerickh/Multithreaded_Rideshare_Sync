#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <queue>
#include <semaphore.h>
#include "ridesharing.h"

/*Consumer
    - defines information to be used by consumer thread*/
struct Consumer{
    //how long in nano seconds to sleep thread per "consumption" of ride request
    int sleep_time;
    //type of consumer as defined by ConsumerType of ridesharing.h
    ConsumerType consumerType;
    /*- address of Broker structure to access requests and update consumer counts
    - imperative that access is used in CRITICAL section*/
    struct Broker* broker;
};

/*Producer
    - define information to be used by producer thread*/
struct Producer{
    /*- how long in nano seconds to sleep thread per "consumption" of ride request*/
    int sleep_time;
    /*- type of consumer as defined by ConsumerType of ridesharing.h*/
    RequestType producerType;
    /*- address of Broker structure to access requests and update consumer counts
    - imperative that access is used in CRITICAL section*/
    struct Broker* broker;
};

/*Request
    - define type of request that, created by Producer*/
struct Request {
    /*- type of request (robot or human) as defined in RequestType in ridesharing.h*/
    RequestType RID; //request id
};

/*Broker
    - shared memory between multiple threads
    - production and consumption totals for each type of Request
    - number of Human or Robot Requests in queue
    - how many and what type of Requests each Consumer consumes
    - enforces exclusive access through mutex
    - enforces Queue capacity and Request type limits through semaphores*/
struct Broker{
    //total number of requests to produce
    int productionLimit;
    int currRequestsProduced;
    int currProduced;
    int currConsumed;

    //- number of Human/Robot Requests currently in queue. [0] = human, [1] = robot
    int inRequestQueue[RequestTypeN];
    //- number of Human/Robot Requests in total. [0] = human, [1] = robot
    int produced[RequestTypeN];
    /*- number of Requests consumed by each Consumer for each Request Type
    - consumed[[numHuman, numRobot], [numHuman, numRobot]]*/
    int* consumed[ConsumerTypeN];

    //SEMAPHORES
    /*- exclusive access to buffer
    - accessing entity waits and then posts*/
    sem_t* Mutex;

    /*- number of requests left in buffer
    - consumer waits, producer posts*/
    sem_t* Unconsumed;

    /*- number of human slots available
    - human producer waits, consumer posts*/
    sem_t* AvailableHumanSlots;

    /*- number of total slots available
    - producer waits, consumer posts*/
    sem_t* AvailableSlots;

    /*- wait for CostAlgo consumer to finish
    - forces main to wait in the meantime*/
    sem_t* BarrierCostAlgo;

    /*- wait for Robo consumer to finish
    - forces main to wait in the meantime*/
    sem_t* BarrierFastAlgo;

    /*- Request holding
    - THE shared piece of memory
    - Consumers add requests 
    - Producers remove requests*/
    std::queue<Request*> buffer;

};

struct Broker* create_Broker(int getN);
struct Consumer* create_Consumer(int sleep_time, ConsumerType consumerType, struct Broker* broker);
struct Producer* create_Producer(int sleep_time, RequestType producerType, struct Broker* broker);
struct Request* create_Request(RequestType request);

#endif