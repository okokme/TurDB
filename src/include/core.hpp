#ifndef CORE_HPP__ 
#define CORE_HPP__ // 第一次没有就定义 
// 保证头在编译的时候只编译一次

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>

#define OSS_MAX_PATHSIZE PATH_MAX   // 路径最大长度
#define OSS_FILE_SEP_STR "/"        // 路径之间的分隔符
#define OSS_FILESEP_CHAR *((const char*)OSS_FILE_SEP_STR)[0]
#define OSS_NEWLINE      "\n"       // 换行符

// error code list
#define EDB_OK                          0  // 正常
#define EDB_IO                          -1 // IO错误
#define EDB_INVALIDARG                  -2 // 非法参数
#define EDB_PERM                        -3 // 权限错误
#define EDB_OOM                         -4 // 内存不够
#define EDB_SYS                         -5 // 系统错误
#define EDB_PMD_HELP_ONLY               -6
#define EDB_PMD_FORCE_SYSTEM_EDU        -7
#define EDB_TIMEOUT                     -8
#define EDB_QUIESCED                    -9
#define EDB_INVAL_STATUS                -10
#define EDB_NETWORK                     -11
#define EDB_NETWORK_CLOSE               -12
#define EDB_APP_FORCED                  -13
#define EDB_IXM_ID_EXIST                -14
#define EDB_HEADER_INVALID              -15
#define EDB_IXM_ID_NOT_EXIST            -16
#define EDB_NO_ID                       -17


#endif


