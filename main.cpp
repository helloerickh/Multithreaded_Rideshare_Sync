#include <unistd.h> //getopt
#include <stdlib.h> //exit
#include <unistd.h> //getopt
#include <stdlib.h> //exit
#include <stdio.h>  //printf
#include <locale>   //isdigit
#include <pthread.h>    //pthreads
#include <semaphore.h>  //semaphores
#include <queue> //queue

#include "structures.h"
#include "thread_functions.h"
#include "io.h"
#include "ridesharing.h"

//default argument values
#define DEFAULT_TOTAL_REQUESTS 120
#define DEFAULT_COST_SAVING_MS 0
#define DEFAULT_FAST_MATCHING_MS 0
#define DEFAULT_HUMAN_REQ_MS 0
#define DEFAULT_ROBOT_REQ_MS 0

//Max for human requests
#define MAX_HUMAN_REQUESTS 4
#define MAX_REQUESTS 12

//SEMAPHORE INITIALIZATION VALS
#define MUTEX 0
#define UNCONSUMED 0
#define MAIN_BARRIER 0

//method headers
void getArguments(int argc, char* argv[], int& getN, int& getC, int& getF, int& getH, int& getR);

int main(int argc, char **argv){

    //total # of requests to generate
    int getN;
    //sleep times for corresponding Consumer threads
    int getC;
    int getF;
    //sleep times for corresponding Producer threads
    int getH;
    int getR;

    //GET ARGUMENTS
    getArguments(argc, argv, getN, getC, getF, getH, getR);

    //STRUCTURE INITIALIZATION
    struct Broker* broker = create_Broker(getN);
    struct Consumer* costConsumer = create_Consumer(getC, CostAlgoDispatch, broker);
    struct Consumer* fastConsumer = create_Consumer(getF, FastAlgoDispatch, broker);
    struct Producer* humanProducer = create_Producer(getH, HumanDriver, broker);
    struct Producer* robotProducer = create_Producer(getR, RoboDriver, broker);
    
    //SEMAPHORE INITIALIZATION
    broker->Mutex = new sem_t;
    broker->Unconsumed = new sem_t;
    broker->AvailableHumanSlots = new sem_t;
    broker->AvailableSlots = new sem_t;
    broker->BarrierCostAlgo = new sem_t;
    broker->BarrierFastAlgo = new sem_t;

    if (sem_init(broker->Mutex, 0, 1) == -1) {
        fprintf(stderr, "Unable to initialize Mutex semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(broker->Unconsumed, 0, UNCONSUMED) == -1) {
        fprintf(stderr, "Unable to initialize Unconsumed semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(broker->AvailableHumanSlots, 0, MAX_HUMAN_REQUESTS) == -1) {
        fprintf(stderr, "Unable to initialize AvailableHumanSlots semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(broker->AvailableSlots, 0, MAX_REQUESTS) == -1) {
        fprintf(stderr, "Unable to initialize AvailableSlots semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(broker->BarrierCostAlgo, 0, MAIN_BARRIER) == -1) {
        fprintf(stderr, "Unable to initialize BarrierCostAlgo semaphore\n");
        exit(EXIT_FAILURE);
    }
    if (sem_init(broker->BarrierFastAlgo, 0, MAIN_BARRIER) == -1) {
        fprintf(stderr, "Unable to initialize BarrierFastAlgo semaphore\n");
        exit(EXIT_FAILURE);
    }

    /*INTIALIZE THREADS*/
    pthread_attr_t pthread_attributes;
    pthread_t costConsumerThread;
    pthread_t fastConsumerThread;
    pthread_t humanProducerThread;
    pthread_t robotProducerThread;

    pthread_attr_init(&pthread_attributes);

    if (pthread_create(&humanProducerThread, &pthread_attributes, &producer_function, (void*) humanProducer)) {
        fprintf(stderr, "Unable to create Cost Algo Consumer thread\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&robotProducerThread, &pthread_attributes, &producer_function, (void*) robotProducer)) {
        fprintf(stderr, "Unable to create Cost Algo Consumer thread\n");
        exit(EXIT_FAILURE);
    }
    //kick off
    if (pthread_create(&costConsumerThread, &pthread_attributes, &consumer_function, (void*) costConsumer)) {
        fprintf(stderr, "Unable to create Cost Algo Consumer thread\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&fastConsumerThread, &pthread_attributes, &consumer_function, (void*) fastConsumer)) {
        fprintf(stderr, "Unable to create Fast Algo Consumer thread\n");
        exit(EXIT_FAILURE);
    }

    /*WAIT FOR CHILD THREADS TO FINISH*/
    sem_wait(broker->BarrierCostAlgo);
    sem_wait(broker->BarrierFastAlgo);

    //IO REPORT CALL
    io_production_report(broker->produced, broker->consumed);

    //CLEANUP
    delete(costConsumer);
    delete(fastConsumer);
    delete(humanProducer);
    delete(robotProducer);
    delete(broker->Mutex);
    delete(broker->Unconsumed);
    delete(broker->AvailableHumanSlots);
    delete(broker->AvailableSlots);
    delete(broker->consumed[CostAlgoDispatch]);
    delete(broker->consumed[FastAlgoDispatch]);

    return 0;
    

}

void getArguments(int argc, char* argv[], int& getN, int& getC, int& getF, int& getH, int& getR){
    //flags to prevent duplicate flag uses
    bool nFlag = false;
    bool cFlag = false;
    bool fFlag = false;
    bool hFlag = false;
    bool rFlag = false;

    //set argument variables to default values 
    getN = DEFAULT_TOTAL_REQUESTS;
    getC = DEFAULT_COST_SAVING_MS;
    getF = DEFAULT_FAST_MATCHING_MS;
    getH = DEFAULT_HUMAN_REQ_MS;
    getR = DEFAULT_ROBOT_REQ_MS;

    //get command line arguments and change argument variables if necessary
    int Option;
    while((Option = getopt(argc, argv, "n:c:f:h:r:")) != -1){
        switch(Option){
            case 'n':
                if(nFlag){
                    printf("ERROR: Multiple -n flags");
                    exit(EXIT_FAILURE);
                }
                else if(atoi(optarg) < 0){
                    printf("Total number of requests must be a number, greater than or equal to 0");
                    exit(EXIT_FAILURE);
                }
                else{
                    getN = atoi(optarg);
                }
                nFlag = true;
                break;
            case 'c':
                if(cFlag){
                    printf("ERROR: Multiple -c flags");
                    exit(EXIT_FAILURE);
                }
                else if(atoi(optarg) < 0){
                    printf("Cost Saving Dispatcher milliseconds must be a number, greater than or equal to 0");
                    exit(EXIT_FAILURE);
                }
                else{
                    getC = atoi(optarg);
                }
                cFlag = true;
                break;
            case 'f':
                if(fFlag){
                    printf("ERROR: Multiple -f flags");
                    exit(EXIT_FAILURE);
                }
                else if(atoi(optarg) < 0){
                    printf("Fast Matching dispatcher milliseconds must be a number, greater than or equal to 0");
                    exit(EXIT_FAILURE);
                }
                else{
                    getF = atoi(optarg);
                }
                fFlag = true;
                break;
            case 'h':
                if(hFlag){
                    printf("ERROR: Multiple -h flags");
                    exit(EXIT_FAILURE);
                }
                else if(atoi(optarg) < 0){
                    printf("Human Driver Ride Request milliseconds must be a number, greater than or equal to 0");
                    exit(EXIT_FAILURE);
                }
                else{
                    getH = atoi(optarg);
                }
                hFlag = true;
                break;
            case 'r':
                if(rFlag){
                    printf("ERROR: Multiple -a flags");
                    exit(EXIT_FAILURE);
                }
                else if(atoi(optarg) < 0){
                    printf("Autonomous Driver Ride Request milliseconds must be a number, greater than or equal to 0");
                    exit(EXIT_FAILURE);
                }
                else{
                    getR = atoi(optarg);
                }
                rFlag = true;
                break;
            case '?':
                printf("ERROR: Unknown argument");
                exit(EXIT_FAILURE);
            default:
                printf("ERROR: Default");
                exit(EXIT_FAILURE);

        }
    }
}    