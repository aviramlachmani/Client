#include <stdlib.h>
#include <mutex>
#include <thread>
#include "../include/Task.h"



/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/


int main (int argc, char *argv[]) {

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);
    BGRS_ConnectionHandler connectionHandler(host,port);
    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }
    std::mutex mutex;
    bool responseWaiting = false;
    bool terminate = false;
    Task read(mutex, responseWaiting,terminate, connectionHandler);
    std::thread th1(&Task::run,&read);

    while (1) {
        if (responseWaiting){
            std::string answer="";
            // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
            // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
            if (!connectionHandler.getLine(answer)) {
                std::lock_guard<std::mutex> lock(mutex);
                std::cout << "trying to get...fail..write again the commend\n" << std::endl;
                responseWaiting = false;
            }else{
                int len=answer.length();
                // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
                // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
                answer.resize(len-1);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    std::cout << answer << std::endl;
                }
                if (answer.compare( "ACK 4")==0) {
                    std::lock_guard<std::mutex> lock(mutex);
                   // std::cout << "Exiting...\n" << std::endl;
                    responseWaiting = false;
                    terminate = true;
                    break;
                }
                responseWaiting=false;
            }
        }
     }
    th1.join();
    return 0;
}
