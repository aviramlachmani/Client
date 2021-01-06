//
// Created by David on 04/01/2021.
//

#include "Task.h"

Task::Task(std::mutex & _mutex,bool &_responseWaiting,bool &_terminate , BGRS_ConnectionHandler & _CH) :
mutex(_mutex), responseWaiting(_responseWaiting), terminate(_terminate), connectionHandler(_CH){}

void Task::run(){
    while(1){
        if (!responseWaiting){
            const short bufsize = 1024;
            char buf[bufsize];
            { //opening new scope so the lock will get destroyed after
                if (terminate) break;
                std::lock_guard<std::mutex> lock(mutex);
                std::cin.getline(buf, bufsize);
            }
            std::string line(buf);
            if (line.compare( "LOGOUT")==0) terminate = true;
            if (!connectionHandler.sendLine(line)) {
                std::lock_guard<std::mutex> lock(mutex);
                std::cout << "trying to send...fail..write again the commend\n" << std::endl;
            }else{responseWaiting = true;}
        }else{std::this_thread::yield();}
    }
}