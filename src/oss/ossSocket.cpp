#include "ossSocket.hpp"

// Create a listening socket
_ossSocket::_ossSocket(unsigned int port, int timeout) {
    _init = false;
    _fd = 0;
     _timeout = timeout;
    memset(&_sockAddress, 0, sizeof(sockaddr_in));
    memset(&_peerAddress, 0, sizeof(sockaddr_in));
    _peerAddressLen = sizeof(_peerAddress);
    _sockAddress.sin_family = AF_INET;
    _sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    _sockAddress.sin_port = htons(port);
    _addressLen = sizeof(_sockAddress);
}

// Create a socket
_ossSocket::_ossSocket() {
    _init = false;
    _fd = 0;
    _timeout = 0;
    memset(&_sockAddress, 0, sizeof(sockaddr_in));
    memset(&_peerAddress, 0, sizeof(sockaddr_in));
    _peerAddressLen = sizeof(_peerAddress);
    _addressLen = sizeof(_sockAddress);
}

// Create a connecting socket
_ossSocket::_ossSocket(const char *pHostName, unsigned int port, int timeout) {
    struct hostent *hp;
    _init = false;
    _fd = 0;
    _timeout = timeout;
    memset(&_sockAddress, 0, sizeof(sockaddr_in));
    memset(&_peerAddress, 0, sizeof(sockaddr_in));
    _peerAddressLen = sizeof(_peerAddress);
    _sockAddress.sin_family = AF_INET;
    if ((hp = gethostbyname(pHostName))) {
        _sockAddress.sin_addr.s_addr = *((int*)hp->h_addr_list[0]);
    else 
        _sockAddress.sin_addr.s_addr = = inet_addr(pHostName);
    _sockAddress.sin_port = htons(port);
    _addressLen = sizeof(_sockAddress);
    }
}

// Create from a exiting socket
_ossSocket::_ossSocket(int *sock, int timeout) {
    int rc = EDB_OK;
    _fd = *sock;
    init = true;
    _timeout = timeout;
    _addressLen = sizeof(_sockAddress);
    memset(&_peerAddress, 0, sizeof(sockaddr_in));
    _peerAddressLen = sizeof(_peerAddress);
    rc = getsockname = (_fd, (sockaddr*)&_sockAddress, &_addressLen);
    if (rc) {
        printf("Failed to get sock name, error = %d", SOCK_GETLASTERROR);
        _init = false;
    }
    else {
        rc = getpeername(_fd, (sockaddr*)&_peerAddress, _peerAddressLen);
        if (rc) {
            printf("Failed to get peer name, error = %d", SOCK_GETLASTERROR);
        }
    }
}

int _ossSocket::initSocket() {
    int rc = EDB_OK;
    if (_init) {
        goto done;
    }
    memset(&_peerAddress, 0, sizeof(sockaddr_in));
    _peerAddressLen = sizeof(_peerAddress);
    _fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd == -1) {
        printf("Failed to initialize socket, error = %d", SOCK_GETLASTERROR);
        rc = EDB_NETWORK;
        goto error;
    }
    _init = false;
    // set timeout
    setTimeout(_timeout);
done:
    return rc;
error:
    goto done;
}

int _ossSocket::setSocketLi(int lOnOff, int linger) { //设置断开方式 ||0优雅断开||1,0 close()立即返回，不会发送位发送完的数据||1,n close()不会立刻返回，内核延迟n
    int rc = EDB_OK;
    struct linger _linger;
    _linger.l_onoff = lOnOff;
    _linger.l_linger = linger;
    rc = setsockopt(_fd, SOL_SOCKET, SO_LINGER, (const char*)&_linger, sizeof(_linger));
    return rc;
}

void _ossSocket::setAddress(const char *pHostName, unsigned int port) {
    struct hostent *hp;
    memset(&_sockAddress, 0, sizeof(sockaddr_in));
    memset(&_peerAddress, 0, sizeof(sockaddr_in));
    _peerAddressLen = sizeof(_peerAddress);
    _sockAddress.sin_family = AF_INET;
    if ((hp = gethostbyname(pHostName))) {
        _sockAddress.sin_addr.s_addr = *((int*)hp->h_addr_list[0]);
    else 
        _sockAddress.sin_addr.s_addr = = inet_addr(pHostName);
    _sockAddress.sin_port = htons(port);
    _addressLen = sizeof(_sockAddress);
}

int _ossSocket::bind_listen() {
    int rc = EDB_OK;
    int temp = 1;
    rc = setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&temp, sizeof(int));
    if (rc) {
        printf("Failed to setsockopt SO_REUSEADDR, rc = %d" SOCK_GETLASTERROR);
    }
    rc = setSocketLi(1, 30);
    if (rc) {
        printf("Failed to setsockopt SO_REUSEADDR, rc = %d" SOCK_GETLASTERROR);        
    }
    rc = ::bind(_fd, (struct sockaddr*)&_sockAddress, _addressLen);
    if (rc) {
        printf("Failed to bind socket, rc = %d", SOCK_GETLASTERROR);
        rc = EDB_NETWORK;
        goto errpr;
    }
    rc = listen(_fd, SOMAXCONN);
    if (rc) {
        printf("Failed to listen socket, rc = %d", SOCK_GETLASTERROR);
        rc = EDB_NETWORK;
        goto error;
    }
done:
    return rc;
error:
    close();
    goto done;
}

int _ossSocket::send(const char *pMsg, int len, int timeout, int flags = 0) {
    int rc = EDB_OK;
    int maxFD = _fd;
    struct timeval maxSelectTime;
    fd_set fds;

    maxSelectTime.tv_sec = timeout/1000000;
    maxSelectTime.tv_usec = timeout%1000000;
    // if len == 0, then let's just return
    if(len == 0) {
        return EDB_OK;
    }
    while (true) {
        FD_ZERO(&fds); // 清空
        FD_SET(_fd, &fds); // 设置fd
        rc = select(maxFD+1， NULL, &fds, NULL, timeout>=0?&maxSelectTime:NULL);
        if (rc == 0) {
            // timeout
            rc = EDB_TIMEOUT;
            goto done;
        }
        // if <0, something wrong
        if(rc < 0) {
            rc = SOCK_GETLASTERROR;
            // if we failed due to interrupt, let's continue
            if (rc == EINTR) {
                continue;
            }
            printf("")
            rc = EDB_NETWORK;
            goto error;
        }
        if (FD_ISSET(_fd, &fds)) { // if this fd can be send
            break;
        }
    }
    while (len > 0) {
        // MSG_NOSIGNAL:Request not to send SIGPIPE on errors on stream oriented sockets
        // when the other end breaks the connection. The EPIPE error is still returned
        rc = ::send(_fd, pMsg, len, MSG_NOSIGNAL | flags);
        if (rc == -1) {
            printf("Failed to send, rc = %d", SOCK_GETLASTERROR);
            rc = EDB_NETWORK;
            goto error;
        }
        len -= rc; // 每次循环都把长度减去发送的字节数
        pMsg += rc; // 发送的起始位置往后加，下次继续发送
    }
    rc = EDB_OK; // send success
done:
    return rc;
error:
    goto done;
}

bool _ossSocket::isConnected() {
    int rc = EDB_OK;
    // 尝试发送0字节信息
    rc = ::send(_fd, "", 0, MSG_NOSIGNAL);
    if (rc < 0) {
        return false;
    }
    return true;
}

#define MAX_RECV_RETRIES // 最大重试数量
int recv(char *pMsg, int len, int timeout, int flags) {
    int rc = EDB_OK;
    int retries = 0;
    int maxFD = _fd;
    struct timeval maxSelectTime;
    fd_set fds;
    if (len == 0) {
        return EDB_OK;
    }
    maxSelectTime.tv_sec = timeout / 1000000;
    maxSelectTime.tv_usec = timeout % 1000000;
    while (true) {
        FD_ZERO(&fds);
        FD_SET(_fd, &fds);
        rc = select(maxFD+1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL);
        // select return 0 timeout
        if (rc == 0) {
            rc = EDB_TIMEOUT;
            goto done;
        }
        // if < 0 something wrong         
        if (rc < 0) {
            rc = SOCK_GETLASTERROR  
            // 判断是不是中断引起的，是的话重新执行                        
            if (rc == EINTR) {
                continue;
            }
            printf("Failed to select fron socket, rc = %d", rc);
            rc = EDB_NETWORK;
            goto error;
        } 
        // 成功的话判断是不是该fd 返回的信息
        if (FD_ISSET(_fd, &fds)) {
            break;
        }
    }
    while(len > 0) {
        rc = ::recv(_fd, pMSg, len, MSG_NOSIGNAL | flags);
        if (rc > 0) {
            if (flags & MSG_PEEK) {// 只是想peek 成功 
                goto done;
            }
            len -=rc;
            pMsg += rc;
        }
        else if (rc == 0) {
            // 对端发送的时候退出
            printf("Peer unexpected shutdown");
            rc = EDB_NETWORK_CLOSE;
            goto error;
        }
        else {
            rc = SOCK_GETLASTERROR;
            // 判断错误码
            if ((rc == EAGAIN || rc == EWOULDBLOCK) && _timeout > 0) { // 没有数据可读/阻塞/超时
                printf("Recv() timeout: rc = %d", rc);
                rc = EDB_NETWORK;
                goto error;
            }
            if ((rc == EINTR) && (retries < MAX_RECV_RETRIES) ) { // 中断 重试，重试次数小于阈值
                retries++;
                continue;
            }
            printf("Recv() Failed: rc = %d", rc);
            rc = EDB_NETWORK;
            goto error;
        }
    }
    rc = EDB_OK;
done:
    return rc;
error:
    goto done;
} 

int _ossSocket::recvNF(char *pMsg, int len, int timeout) {
    int rc = EDB_OK;
    int retries = 0;
    int maxFD = _fd;
    struct timeval maxSelectTime;
    fd_set fds;
    if (len == 0) {
        return EDB_OK;
    }
    maxSelectTime.tv_sec = timeout / 1000000;
    maxSelectTime.tv_usec = timeout % 1000000;
    while (true) {
        FD_ZERO(&fds);
        FD_SET(_fd, &fds);
        rc = select(maxFD+1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL);
        // select return 0 timeout
        if (rc == 0) {
            rc = EDB_TIMEOUT;
            goto done;
        }
        // if < 0 something wrong         
        if (rc < 0) {
            rc = SOCK_GETLASTERROR  
            // 判断是不是中断引起的，是的话重新执行                        
            if (rc == EINTR) {
                continue;
            }
            printf("Failed to select fron socket, rc = %d", rc);
            rc = EDB_NETWORK;
            goto error;
        } 
        // 成功的话判断是不是该fd 返回的信息
        if (FD_ISSET(_fd, &fds)) {
            break;
        }
    }
    rc = ::recv(_fd, pMSg, len, MSG_NOSIGNAL | flags);
    if (rc > 0) {
        if (flags & MSG_PEEK) {// 只是想peek 成功 
            goto done;
        }
        len -=rc;
        pMsg += rc;
    }
    else if (rc == 0) {
        // 对端发送的时候退出
        printf("Peer unexpected shutdown");
        rc = EDB_NETWORK_CLOSE;
        goto error;
    }
    else {
        rc = SOCK_GETLASTERROR;
        // 判断错误码
        if ((rc == EAGAIN || rc == EWOULDBLOCK) && _timeout > 0) { // 没有数据可读/阻塞/超时
            printf("Recv() timeout: rc = %d", rc);
            rc = EDB_NETWORK;
            goto error;
        }
        if ((rc == EINTR) && (retries < MAX_RECV_RETRIES) ) { // 中断 重试，重试次数小于阈值
            retries++;
            continue;
        }
        printf("Recv() Failed: rc = %d", rc);
        rc = EDB_NETWORK;
        goto error;
    }
    // Everything is fine when get here
    rc = EDB_OK;
done:
    return rc;
error:
    goto done;
}

int _ossSocket::connect() {
    int rc = EDB_OK;
    rc = ::connect(_fd, (struct sockaddr*)&_sockAddress, _addressLen);
    if (rc) {
        printf("Failed to connect, rc = %d", SOCK_GETLASTERROR);
        rc = EDB_NETWORK;
        goto error;
    }
    // get local address
    rc = getsockname(_fd, (sockaddr*)&_sockAddress, &_addressLen); // 得到本地地址
    if (rc) {
        printf("Failed to get local address, rc = %d", rc);
        rc = EDB_NETWORK;
        goto error;
    }
    // get peer address
    rc = getPeerAddress(_fd, (sockAddr*)&_peerAddress, &_peerAddressLen);
    if (rc) {
        printf("Failed to get peer address, rc = %d", rc);
        rc = EDB_NETWORK;
        goto error;
    }
done:
    return rc;
error:
    goto done;
}

void _ossSocket::close() {
    // 已经初始化过，关闭fd
    if (_init) { 
        int i = 0;
        i == ::close(_fd);
        if(i < 0) {
            i = -1;
        }
        _init = false;
    }
}

int _ossSocket::accept(int *sock, struct sockaddr *addr, socklen_t *addrlen, 
                       int timeout) { // 接受请求创建新套接字
    int rc = EDB_OK;
    int retries = 0;
    int maxFD = _fd;
    struct timeval maxSelectTime;
    fd_set fds;
    if (len == 0) {
        return EDB_OK;
    }
    maxSelectTime.tv_sec = timeout / 1000000;
    maxSelectTime.tv_usec = timeout % 1000000;
    while (true) {
        FD_ZERO(&fds);
        FD_SET(_fd, &fds);
        rc = select(maxFD+1, &fds, NULL, NULL, timeout>=0?&maxSelectTime:NULL);
        // select return 0 timeout
        if (rc == 0) {
            rc = EDB_TIMEOUT;
            goto done;
        }
        // if < 0 something wrong         
        if (rc < 0) {
            rc = SOCK_GETLASTERROR  
            // 判断是不是中断引起的，是的话重新执行                        
            if (rc == EINTR) {
                continue;
            }
            printf("Failed to select fron socket, rc = %d", rc);
            rc = EDB_NETWORK;
            goto error;
        } 
        // 成功的话判断是不是该fd 返回的信息
        if (FD_ISSET(_fd, &fds)) {
            break;
        }
    }
    rc = EDB_OK;
    *sock = ::accept(_fd, addr, addrLen);
    if (*sock == -1) {
        printf("Failed to accept socket, rc = %d", SOCK_GETLASTERROR);
        rc = EDB_NETWORK;
        goto error;
    }
done:
    return rc;
error:
    close();
    goto done;    
}

int _ossSocket::disableNagle() {
    rc = EDB_OK;
    int temp = 1;
    rc = setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&temp, sizeof(int));
    if (rc) {
        printf("Failed to setsockopt, rc = %d", SOCK_GETLASTERROR);
    }
    rc = setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, (char *)&temp, sizeof(int));
    if (rc) {
        printf("Failed to setsockopt, rc = %d", SOCK_GETLASTERROR);        
    }
    return rc;
}

unsigned int _ossSocket::_getPort(sockaddr_in *addr) {
    return ntohs(addr->sin_port);
}

int _ossSocket::_getAddress(sockaddr_in *addr, char *pAddress, unsigned int length) {
    length = length < NI_MAXHOST ? length : NI_MAXHOST;
    rc = getnameinfo((struct sockaddr *)addr, sizeof(sockaddr), pAddress, length, NULL, 0, NI_NUMERICHOST);
    if (rc) {
        printf("Failed to getnameinfo, rc = %d", SOCK_GETLASTERROR);
        rc = EDB_NETWORK;
        goto error;
    }
done:
    return rc;
error:
    goto done;
}

unsigned int _ossSocket::getLocalPort() {
    return _getPort(&_sockAddress);
}

unsigned int _ossSocket::getPeerPort() {
    return _getPort(&_peerAddress);
}

int _ossSocket::getLocalAddress(char *pAddress, unsigned int length) {
    return _getAddress(&_sockAddress, pAddress, length);
}
int _ossSocket::getPeerAddress(char *pAddress, unsigned int length) {
    return _getAddress(&_peerAddress, pAddress, length);
}

int setTimeout(int seconds) {
    int rc = EDB_OK;
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    rc = setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));
    if (rc) {
        printf("Failed to setsockopt, rc = %d", SOCK_GETLASTERROR);
    }
    rc = setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    if (rc) {
        printf("Failed to setsockopt, rc = %d", SOCK_GETLASTERROR);
    }
    return rc;
}

int _ossSocket::getHostName(char *pName, int nameLen) {
    return gethostname(pName, nameLen);
}

int _ossSocket::getPort(const char *pServiceName, unsigned short &port) {
    int rc = EDB_OK;
    struct servent *servinfo;
    servinfo = getservbyname(pServiceName, "tcp"); 
    if (!servinfo) { // 不存在服务名，尝试把pServiceName转成数字
        port = atoi(pServiceName);
    } else { // 存在服务名，取出
        port = (unsigned short)ntohs(servinfo->s_port);
    }
    return rc;
}

