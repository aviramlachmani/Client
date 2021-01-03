#include <stdlib.h>
#include <mutex>
#include <thread>
//#include "../include/Task.h"
#include "../include/BGRS_ConnectionHandler.h"


/**
* This code assumes that the server replies the exact text the client sent it (as opposed to the practical session example)
*/
class Task{
private:
    std::mutex & mutex;
    bool & responseWaiting;
    bool & terminate;
    BGRS_ConnectionHandler & connectionHandler;
public:
    Task(std::mutex & _mutex,bool &_responseWaiting,bool &_terminate , BGRS_ConnectionHandler & _CH) :
            mutex(_mutex), responseWaiting(_responseWaiting), terminate(_terminate), connectionHandler(_CH){}

    void run(){
        while(1){
            if (!responseWaiting){
                const short bufsize = 1024;
                char buf[bufsize];
                { //opening new scope so the lock will get destroyed after
                    if (terminate) break;
                    std::lock_guard<std::mutex> lock(mutex);
                    std::cout<<"Client>" <<std::flush;
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
};


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
    int expectedResponses = 0;
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
                    std::cout << "Client<"+answer << std::endl;
                }
                if (answer.compare( "ACK 4")==0) {
                    std::lock_guard<std::mutex> lock(mutex);
                    std::cout << "Exiting...\n" << std::endl;
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
