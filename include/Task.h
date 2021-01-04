//
// Created by David on 04/01/2021.
//

#ifndef CLIENT_TASK_H
#define CLIENT_TASK_H

#include <stdlib.h>
#include <mutex>
#include <thread>
#include "BGRS_ConnectionHandler.h"

class Task{
private:
    std::mutex & mutex;
    bool & responseWaiting;
    bool & terminate;
    BGRS_ConnectionHandler & connectionHandler;
public:
    Task(std::mutex & _mutex,bool &_responseWaiting,bool &_terminate , BGRS_ConnectionHandler & _CH);
    void run();
};


#endif //CLIENT_TASK_H
