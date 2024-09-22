#include "structures.h"

Broker* create_Broker(int getN){
    struct Broker* broker = new Broker();
    broker->productionLimit = getN;
    broker->currRequestsProduced = 0;
    broker->currProduced = 0;
    broker->currConsumed = 0;

    for(int i = 0; i < ConsumerTypeN; i++){
        broker->inRequestQueue[i] = 0;
        broker->produced[i] = 0;
    }

    for(int i = 0; i < ConsumerTypeN; i++){
        broker->consumed[i] = new int[RequestTypeN]();
    }

    return broker;
}

Consumer* create_Consumer(int sleep_time, ConsumerType consumerType, struct Broker* broker){
    struct Consumer* consumer = new Consumer();
    consumer->sleep_time = sleep_time;
    consumer->consumerType = consumerType;
    consumer->broker = broker;
    return consumer;
}

Producer* create_Producer(int sleep_time, RequestType producerType, struct Broker* broker){
    struct Producer* producer = new Producer();
    producer->sleep_time = sleep_time;
    producer->producerType = producerType;
    producer->broker = broker;
    return producer;
}

Request* create_Request(RequestType requestType){
    struct Request* request = new Request();
    request->RID = requestType;
    return request;
}

