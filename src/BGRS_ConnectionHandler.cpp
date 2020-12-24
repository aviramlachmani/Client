#include <BGRS_ConnectionHandler.h>

using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

BGRS_ConnectionHandler::BGRS_ConnectionHandler(string host, short port): host_(host), port_(port), io_service_(), socket_(io_service_){}

BGRS_ConnectionHandler::~BGRS_ConnectionHandler() {
    close();
}

bool BGRS_ConnectionHandler::connect() {
    std::cout << "Starting connect to "
              << host_ << ":" << port_ << std::endl;
    try {
        tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
        boost::system::error_code error;
        socket_.connect(endpoint, error);
        if (error)
            throw boost::system::system_error(error);
    }
    catch (std::exception& e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool BGRS_ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp ) {
            tmp += socket_.read_some(boost::asio::buffer(bytes+tmp, bytesToRead-tmp), error);
        }
        if(error)
            throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool BGRS_ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    int tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp ) {
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
        if(error)
            throw boost::system::system_error(error);
    } catch (std::exception& e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool BGRS_ConnectionHandler::getLine(std::string& line) {
    return getFrameAscii(line, '\n');
}

bool BGRS_ConnectionHandler::sendLine(std::string& line) {
    std::string commend;
    std::string rest;
    for(int i=0; i<line.length();i++){
        if(line[i]==' ') {
            commend=line.substr(0,i);
            rest=line.substr(i+1);
            break;
        }
    }
    if(commend=="ADMINREG") return sendFrameAscii(1,rest,'\n');
    if(commend=="STUDENTREG") return sendFrameAscii(2,rest,'\n');
    if(commend=="LOGIN") return sendFrameAscii(3,rest,'\n');
    if(commend=="LOGOUT") return sendFrameAscii(4,rest,'\n');
    if(commend=="COURSEREG") return sendFrameAscii(5,rest,'\n');
    if(commend=="KDAMCHECK") return sendFrameAscii(6,rest,'\n');
    if(commend=="COURSESTAT") return sendFrameAscii(7,rest,'\n');
    if(commend=="STUDENTSTAT") return sendFrameAscii(8,rest,'\n');
    if(commend=="ISREGISETERD") return sendFrameAscii(9,rest,'\n');
    if(commend=="UNREGISTERD") return sendFrameAscii(10,rest,'\n');
    if(commend=="MYCOURSE") return sendFrameAscii(11,rest,'\n');
    else std::cerr << "recv failed (Error: illegal commend )" << std::endl;
    return false;
}

bool BGRS_ConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    char commend[2];
    try {
        for(int i=0;i<2;i++) {
            if (!getBytes(&ch, 1)) ////read the commend
            {
                return false;
            }
            commend[i]=ch;
        }
        int result = int((unsigned char)(0) << 24 |  ///convert the commend to int
                    (unsigned char)(0) << 16 |
                    (unsigned char)(commend[0]) << 8 |
                    (unsigned char)(commend[1]));


        if(result==12) frame.append("ACK "); ///convert the short to string
        else if(result==13) frame.append("ERROR "); ///convert the short to string
        else {
            return false;
        }

        for(int i=0;i<2;i++) {
            if (!getBytes(&ch, 1)) ////read the message opcode
            {
                return false;
            }
            commend[i]=ch;
        }
        int result1 = int((unsigned char)(0) << 24 |  ///convert the commend to int
                         (unsigned char)(0) << 16 |
                         (unsigned char)(commend[0]) << 8 |
                         (unsigned char)(commend[1]));
        frame.append(std::to_string(result1)+" ");



        if(result1==6) return makeKedmCheckMassage(frame,delimiter);
        if(result1==7) return makeCourseStat(frame,delimiter);              ///todo there are problem with the answer of the server
        if(result1==8) return makeStudentStat(frame,delimiter);
        if(result1==9) return makeIsRegistered(frame,delimiter);

        if (!getBytes(&ch, 1)) /// read the last baye '\n'
        {
            return false;
        }


    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}


bool BGRS_ConnectionHandler::sendFrameAscii(short commend,const std::string& frame, char delimiter) {
    char commendByte[2];
    commendByte[0] = ((commend >> 8) & 0xFF);
    commendByte[1] = (commend & 0xFF);
    bool result1=sendBytes(commendByte,2);
    bool result=sendBytes(frame.c_str(),frame.length());
    if(!result || !result1) return false;
    return sendBytes(&delimiter,1);
}

// Close down the connection properly.
void BGRS_ConnectionHandler::close() {
    try{
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}

bool BGRS_ConnectionHandler::makeKedmCheckMassage(string &frame, char delimiter) {
    frame=frame+"\n[";
    char ch;
    char courseNum[2];
    try {
    do{
        for(int i=0;i<2;i++) {
            if (!getBytes(&courseNum[i], 1)) {
                return false;
            }
            if (courseNum[i] == delimiter){
                frame=frame+"]";
                return true;
            };
        }
        int result1 = int((unsigned char)(0) << 24 |  ///convert the commend to int
                         (unsigned char)(0) << 16 |
                         (unsigned char)(courseNum[0]) << 8 |
                         (unsigned char)(courseNum[1]));
        frame.append(std::to_string(result1)+",");

        if (!getBytes(&ch, 1)) {  /// read the '\n'
            return false;
        }
        }while (1);
      } catch (std::exception& e) {
    std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
    return false;
    }
}

bool BGRS_ConnectionHandler::makeIsRegistered(string &frame, char delimiter) {
    frame=frame+"\n";
    char ch;
    try {
        do{
            if(!getBytes(&ch, 1))
            {
                return false;
            }
            if(ch!='\0')
                frame.append(1, ch);
        }while (delimiter != ch);
    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool BGRS_ConnectionHandler::makeStudentStat(string &frame, char delimiter) {
    frame=frame+"\nStudent: ";
    char ch;
    try {
        do{
            if(!getBytes(&ch, 1))
            {
                return false;
            }
            if(ch!='\0')frame.append(1, ch);
            else {
                frame=frame+"\nCourses: ";
                return makeKedmCheckMassage(frame,delimiter);
            }
        }while (delimiter != ch);

    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool BGRS_ConnectionHandler::makeCourseStat(string &frame, char delimiter) {
    frame=frame+"\nCourse: ";
    char courseNum[2];
    char ch;
    try {
        for (int i = 0; i < 2; i++) {
            if (!getBytes(&courseNum[i], 1)) {
                return false;
            }
            if (courseNum[i] == delimiter) {
                frame = frame + "]";
                return true;
            };
        }
        int result1 = int((unsigned char)(0) << 24 |  ///convert the commend to int
                          (unsigned char)(0) << 16 |
                          (unsigned char)(courseNum[0]) << 8 |
                          (unsigned char)(courseNum[1]));
        frame.append("("+std::to_string(result1) + ")");

        do{
            if(!getBytes(&ch, 1))
            {
                return false;
            }
            if(ch!='\0')frame.append(1, ch);
        }while (ch!='\0');
        frame=frame+"\nSeats Available: ";

        for (int i = 0; i < 2; i++) {                ///read the seats that available
            if (!getBytes(&courseNum[i], 1)) return false;}

        int result2 = int((unsigned char)(0) << 24 |  ///convert the commend to int
                          (unsigned char)(0) << 16 |
                          (unsigned char)(courseNum[0]) << 8 |
                          (unsigned char)(courseNum[1]));
        frame.append(std::to_string(result2)+'/');

        for (int i = 0; i < 2; i++) {                ///read the seats that available
            if (!getBytes(&courseNum[i], 1)) return false;}

        short result3 = (short) ((courseNum[0] & 0xff) << 8); ///convert to courseNum to short
        result3 += (short) (courseNum[1] & 0xff);
        frame.append(std::to_string(result3)+"\n[");

        do{
            if(!getBytes(&ch, 1))
                {
                    return false;
                }
                if(ch!='\0')frame.append(1, ch);
                else {
                    frame=frame+",";
                    return makeKedmCheckMassage(frame,delimiter);
                }
            }while (delimiter != ch);
            frame=frame.substr(0,frame.length()-1);
            frame=frame+"]";

        } catch (std::exception& e) {
    std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
    return false;}
    return true;
}

