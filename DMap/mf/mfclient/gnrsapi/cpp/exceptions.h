#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

using namespace std;

namespace network_exceptions {

    class NetException : public exception {
        public:
            NetException(string s="net error"):msg(s){}
            ~NetException() throw() {}
            const char* what() const throw() { return msg.c_str(); }
        protected: 
            string msg;
    };

    class NetAddrTypeException : public NetException {
        public:
            NetAddrTypeException(string s="netaddr type error"):
                    NetException(s){}
    };
    class ResolveException : public NetException {
        public:
            ResolveException(string s="resolve error"):
                    NetException(s){}
    };
    class BindException : public NetException {
        public:
            BindException(string s="bind error"):
                    NetException(s){}
    };
    class SocketException : public NetException {
        public:
            SocketException(string s="socket error"):
                    NetException(s){}
    };
    class NetIOException : public NetException {
        public:
            NetIOException(string s="net send/recv error"):
                    NetException(s){}
    };

    class TimeoutException : public NetException {
        public:
            TimeoutException(string s="operation timed out"):
                    NetException(s){}
    };


}
#endif //EXCEPTIONS_H
