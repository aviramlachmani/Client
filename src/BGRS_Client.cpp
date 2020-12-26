#include <stdlib.h>
#include "../include/BGRS_ConnectionHandler.h"


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

    //From here we will see the rest of the ehco client implementation:
    while (1) {
        bool canSend=true;
        bool ans=true;
        const short bufsize = 1024;
        char buf[bufsize];
        std::cout<<"Client>" <<std::flush;
        std::cin.getline(buf, bufsize);
        std::string line(buf);

        int len=line.length();
        if (!connectionHandler.sendLine(line)) {
            std::cout << "trying to send...fail..write again the commend\n" << std::endl;
            canSend=false;
        }

        std::string answer="";
        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
        if (canSend && !connectionHandler.getLine(answer)) {
            std::cout << "trying to get...fail..write again the commend\n" << std::endl;
            ans= false;
        }
        if(ans){

          len=answer.length();
           // A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
           // we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
           answer.resize(len-1);

        std::cout << "Client<"+answer << std::endl;
        if (answer.compare( "ACK 4")==0) {
            std::cout << "Exiting...\n" << std::endl;
            break;
        }
     }
    }
    return 0;
}
