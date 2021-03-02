#ifndef OSSSOCKET_HPP__
#define OSSSOCKET_HPP__

#define "core.hpp"
#define SOCK_GETLASTERROR errno

// by default 10ms timeout
#define OSS_SOCKET_DFT_TIMEOUT 10000

// max hostname
#define OSS_MAX_HOSTNAME NI_MAXHOST
#define OSS_MAX_SERVICENAME NI_MAXSERV

class _ossSocket
{
private:
    int _fd;
    socklen_t _addressLen;
    socklen_t _peerAddressLen;
    struct sockaddr_in _sockAddress;
    struct sockaddr_in _peerAddress;
    bool _init;
    int _timeout;
protected:
    unsigned int _getPort(sockaddr_in *addr);
    int _getAddress(sockaddr_in *addr, char *pAddress, unsigned int length);
public:
    int setSocketLi(int lOnOff, int linger); // 设置超时断开
    void setAddress(const char *pHostName, unsigned int port);
    // create a listening socket
    _ossSocket();
    _ossSocket(unsigned int port, int timeout = 0);
    // create a connection socket
    _ossSocket(const char *pHostName, unsigned int port, int timeout = 0);
    // create from a exiting socket
    _ossSocket(int *socket, int timeout = 0);
    ~_ossSocket(){
        close();
    }
    int initSocket();
    int bind_listen();
    int send(const char *pMsg, int len,
             int timeout = OSS_SOCKET_DFT_TIMEOUT,
             int flags = 0);
    bool isConnected();
    int recv(char *pMsg, int len,
             int timeout = OSS_SOCKET_DFT_TIMEOUT,
             int flags = 0); // 收到len长度字符才会返回
    int recvNF(char *pMsg, int len,
               int timeout = OSS_SOCKET_DFT_TIMEOUT); //有消息就会返回
    int connect();
    void close();
    int accept(int *sock, struct sockaddr *addr, socklen_t *addrlen, 
               int timeout = OSS_SOCKET_DFT_TIMEOUT);
    int disableNagle(); // tcp中当我们发送小包时会等待汇成一个大包发送，减少打包时间。但是数据库不需要，可能就是需要实时发送小包，所以关闭此特性。
    unsigned int getPeerPort();
    int getPeerAddress(char *pAddress, unsigned int length);
    unsigned int getLocalPort(); // 得到本地端口
    int getLocalAddress(char *pAddress, unsigned int length); // 得到远程端口
    int setTimeout(int seconds);
    static int getHostName(char *pName, int nameLen);
    static int getPort(const char *pServiceName, unsigned short &port); // 把服务名转化为端口号
}
typedef class _ossSocket ossSocket;

#endif
