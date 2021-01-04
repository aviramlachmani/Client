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
    std::cout << "Starting connect to "<< host_ << ":" << port_ << std::endl;
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
    return getFrameAscii(line, '/');
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
    if(commend.empty()){
        commend=line;
        rest="";
    }
    if(commend=="ADMINREG") return sendFrameAscii(1,rest,'/', false);
    if(commend=="STUDENTREG") return sendFrameAscii(2,rest,'/', false);
    if(commend=="LOGIN") return sendFrameAscii(3,rest,'/', false);
    if(commend=="LOGOUT") return sendFrameAscii(4,rest,'/', false);
    if(commend=="COURSEREG") return sendFrameAscii(5,rest,'/',true);
    if(commend=="KDAMCHECK") return sendFrameAscii(6,rest,'/',true);
    if(commend=="COURSESTAT") return sendFrameAscii(7,rest,'/',true);
    if(commend=="STUDENTSTAT") return sendFrameAscii(8,rest,'/', false);
    if(commend=="ISREGISTERED") return sendFrameAscii(9,rest,'/',true);
    if(commend=="UNREGISTER") return sendFrameAscii(10,rest,'/',true);
    if(commend=="MYCOURSES") return sendFrameAscii(11,rest,'/', false);
    else  return sendFrameAscii(13,rest,'/', false);
}

bool BGRS_ConnectionHandler::getFrameAscii(std::string& frame, char delimiter) {
    char ch;
    char com[2];
    string str;
    string num;
    try {
       for(int i=0;i<2;i++){
            if (!getBytes(&ch, 1)) ////read the commend
            {
                return false;
            }
           com[i]=ch;
        }
        short result = bytesToShort(com);
        if(result==12) frame.append("ACK "); ///convert the short to string
        else if(result==13) frame.append("ERROR "); ///convert the short to string
        else {return false;}

        for(int i=0;i<2;i++){
            if (!getBytes(&ch, 1)) ////read the commend
            {
                return false;
            }
            com[i]=ch;
        }
        short result1=bytesToShort(com);
        frame.append(std::to_string(result1)+" ");

        if(result1==6 && result==12) return makeKedmCheckMassage(frame,delimiter,true);
        if(result1==7 && result==12) return makeCourseStat(frame,delimiter);
        if(result1==8 && result==12) return makeStudentStat(frame,delimiter);
        if(result1==9 && result==12) return makeIsRegistered(frame,delimiter);
        if(result1==11 && result==12) return makeKedmCheckMassage(frame,delimiter,true);

        if (!getBytes(&ch, 1)) /// read the last byte '/'
        {return false;}

    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}


bool BGRS_ConnectionHandler::sendFrameAscii(short commend, std::string& frame, char delimiter,bool ans) {
    char com[2];
    shortToBytes(commend,com);
    bool result1=sendBytes(com,2);
    bool result;
    if(ans){
        shortToBytes(ReadNext2Bytes(frame),com);
        result=sendBytes(com,2);
    }
    else{
        for(int i=0;i<frame.length();i++){
            if(frame[i]==' ')frame[i]='\0';
        }
       result=sendBytes(frame.c_str(),frame.length());
    }
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

bool BGRS_ConnectionHandler::makeKedmCheckMassage(string &frame, char delimiter,bool a) {
    if(a)frame=frame+"\n[";
    bool ans=false;
    char ch;
    try {
        do{if (!getBytes(&ch, 1)) ////read the commend
            {
                return false;
            }
            if(ch!=' ')  frame.append(1, ch);
             else { ans=true;
                 frame.append(",");
             }
        }while (ch!=delimiter);
       if(ans) frame.resize(frame.length()-2);
       else frame.resize(frame.length()-1);
        frame=frame+"] ";
      } catch (std::exception& e) {
    std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
    return false;
    }
    return true;
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
                frame=frame+"\nCourses: [";
                return makeKedmCheckMassage(frame,delimiter, false);
            }
        }while (delimiter != ch);

    } catch (std::exception& e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool BGRS_ConnectionHandler::makeCourseStat(string &frame, char delimiter) {
    frame=frame+"\nCourse: (";
    char ch;
    try {
        do{
            if (!getBytes(&ch, 1)) {/// read the number of course
                return false;
            }
           if(ch!=' ') frame.append(1,ch);
        }while (ch!=' ');
        frame=frame+") ";
        do{
            if (!getBytes(&ch, 1)) {/// read the name of course
                return false;
            }
            frame.append(1,ch);
        }while (ch!='\0');
        frame=frame+"\nSeats Available: ";
        do{
            if (!getBytes(&ch, 1)) {/// read the Current seats that taken of course
                return false;
            }
            if(ch!=' ') frame.append(1,ch);
        }while (ch!=' ');
        frame=frame+"/";
        do{
            if (!getBytes(&ch, 1)) {/// read the Max seats of course
                return false;
            }
            frame.append(1,ch);
        }while (ch!=' ');
        bool ans=false;
        frame=frame+"\nStudent Registered: [";
        do{
            if(!getBytes(&ch, 1))
                {
                    return false;
                }
                if(ch!='\0')frame.append(1, ch);
                else {
                    frame=frame+",";
                    ans=true;
                }
            }while (delimiter != ch);
           if(ans) frame=frame.substr(0,frame.length()-2);
           else frame=frame.substr(0,frame.length()-1);
            frame=frame+"] ";

        } catch (std::exception& e) {
    std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
    return false;}
    return true;
}
short BGRS_ConnectionHandler:: bytesToShort(char* bytesArr)
{
    short result = (short)((bytesArr[0] & 0xff) << 8);
    result += (short)(bytesArr[1] & 0xff);
    return result;
}
void BGRS_ConnectionHandler:: shortToBytes(short num, char* bytesArr)
{
    bytesArr[0] = ((num >> 8) & 0xFF);
    bytesArr[1] = (num & 0xFF);
}
short BGRS_ConnectionHandler:: ReadNext2Bytes(string &line){
    short commend=0;
    for(int i=0; i<line.length();i++){
        if(line[i]==' ') {
            commend=std::stoi( line.substr(0,i));
            line=line.substr(i+1);
            break;
        }
    }
    if(commend==0){
        commend=std::stoi(line);
        line="";
    }
    return commend;
}



