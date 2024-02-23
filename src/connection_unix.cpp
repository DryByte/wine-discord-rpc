#include "connection.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define AF_UNIX 1
#define F_SETFL 4
#define O_NONBLOCK 00004000
#define EAGAIN -11

struct unix_sockaddr {
    short sa_family;
    char sa_data[108];
};

int unix_socket()
{
    int ret = -1;

    asm volatile("mov $359, %%eax\n\t" // sys_socket
                 "mov $1, %%ebx\n\t"   // AF_UNIX
                 "mov $1, %%ecx\n\t"   // SOCK_STREAM
                 "mov $0, %%edx\n\t"
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 :);

    return ret;
}

int fcntl(unsigned int fd, unsigned int cmd, unsigned long arg)
{
    int ret = -1;

    asm volatile("mov $55, %%eax\n\t" // sys_fcntl
                 "mov %1, %%ebx\n\t"  // fd
                 "mov %2, %%ecx\n\t"  // cmd
                 "mov %3, %%edx\n\t"  // arg
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(fd), "m"(cmd), "m"(arg));

    return ret;
}

int unix_setsockopt(int socket,
                    int level,
                    int option_name,
                    const void* option_value,
                    size_t option_len)
{
    int ret = -1;

    asm volatile("mov $366, %%eax\n\t" // sys_setsockopt
                 "mov %1, %%ebx\n\t"   // socket
                 "mov %2, %%ecx\n\t"   // level
                 "mov %3, %%edx\n\t"   // option_name
                 "mov %4, %%esi\n\t"   // option_value
                 "mov %5, %%edi\n\t"   // option_len
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(socket), "m"(level), "m"(option_name), "m"(option_value), "m"(option_len));

    return ret;
}

int unix_connect(int sockfd, const unix_sockaddr* addr, size_t len)
{
    int ret = -1;

    asm volatile("mov $362, %%eax\n\t" // sys_connect
                 "mov %1, %%ebx\n\t"   // sockfd
                 "mov %2, %%ecx\n\t"   // addr
                 "mov %3, %%edx\n\t"   // len
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(sockfd), "m"(addr), "m"(len));

    return ret;
}

int unix_close(int sockfd)
{
    int ret = -1;

    asm volatile("mov $6, %%eax\n\t" // sys_connect
                 "mov %1, %%ebx\n\t" // sockfd
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(sockfd));

    return ret;
}

/*int unix_send(int sockfd, const void* buf, size_t len, int flags)
{
    int ret = 0;
    asm volatile("mov $369, %%eax\n\t" // sys_sendto
                 "mov %1, %%ebx\n\t"   // sockfd
                 "mov %2, %%ecx\n\t"   // buf
                 "mov %3, %%edx\n\t"   // len
                 "mov %4, %%esi\n\t"   // flags
                 "mov $0, %%edi\n\t"   // dest_addr
                 "mov $0, %%ebp\n\t"   // addrlen
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(sockfd), "m"(buf), "m"(len), "m"(flags));

    return ret;
}*/

int unix_write(int sockfd, const void* buf, size_t len)
{
    int ret = 0;
    asm volatile("mov $4, %%eax\n\t" // sys_write
                 "mov %1, %%ebx\n\t" // sockfd
                 "mov %2, %%ecx\n\t" // buf
                 "mov %3, %%edx\n\t" // len
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(sockfd), "m"(buf), "m"(len));

    return ret;
}

/*int unix_recv(int sockfd, const void* buf, size_t len, int flags)
{
    int ret = 0;
    asm volatile("mov $371, %%eax\n\t" // sys_recvfrom
                 "mov %1, %%ebx\n\t"   // sockfd
                 "mov %2, %%ecx\n\t"   // buf
                 "mov %3, %%edx\n\t"   // len
                 "mov %4, %%esi\n\t"   // flags
                 "mov $0, %%edi\n\t"   // src_addr
                 "mov $0, %%ebp\n\t"   // addrlen
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(sockfd), "m"(buf), "m"(len), "m"(flags));

    return ret;
}*/

int unix_read(int sockfd, const void* buf, size_t len)
{
    int ret = 0;
    asm volatile("mov $3, %%eax\n\t" // sys_read
                 "mov %1, %%ebx\n\t" // sockfd
                 "mov %2, %%ecx\n\t" // buf
                 "mov %3, %%edx\n\t" // len
                 "int $0x80\n\t"
                 "mov %%eax, %0"
                 : "=r"(ret)
                 : "m"(sockfd), "m"(buf), "m"(len));
    return ret;
}

static struct unix_sockaddr PipeAddr;

#ifdef MSG_NOSIGNAL
static int MsgFlags = MSG_NOSIGNAL;
#else
static int MsgFlags = 0;
#endif

static const char* GetTempPath()
{
    const char* temp = getenv("XDG_RUNTIME_DIR");
    temp = temp ? temp : getenv("TMPDIR");
    temp = temp ? temp : getenv("TMP");
    temp = temp ? temp : getenv("TEMP");
    temp = temp ? temp : "/tmp";
    return temp;
}

/*static*/ /*BaseConnection* BaseConnection::Create()
{
    PipeAddr.sun_family = AF_UNIX;
    return &Connection;
}*/

/*static*/ void BaseConnectionUnix::Destroy(BaseConnection*& c)
{
    auto self = reinterpret_cast<BaseConnectionUnix*>(c);
    self->Close();
    c = nullptr;
}

bool BaseConnectionUnix::Open()
{
    PipeAddr.sa_family = AF_UNIX;
    const char* tempPath = GetTempPath();
    auto self = reinterpret_cast<BaseConnectionUnix*>(this);
    self->sock = unix_socket();
    if (self->sock == -1) {
        return false;
    }
    fcntl(self->sock, F_SETFL, O_NONBLOCK);
#ifdef SO_NOSIGPIPE
    int optval = 1;
    unix_setsockopt(self->sock, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval));
#endif

    for (int pipeNum = 0; pipeNum < 10; ++pipeNum) {
        snprintf(
          PipeAddr.sa_data, sizeof(PipeAddr.sa_data), "%s/discord-ipc-%d", tempPath, pipeNum);

        int err = unix_connect(self->sock, &PipeAddr, sizeof(PipeAddr));

        if (err == 0) {
            self->isOpen = true;
            return true;
        }
    }
    self->Close();
    return false;
}

bool BaseConnectionUnix::Close()
{
    auto self = reinterpret_cast<BaseConnectionUnix*>(this);
    if (self->sock == -1) {
        return false;
    }
    unix_close(self->sock);
    self->sock = -1;
    self->isOpen = false;
    return true;
}

bool BaseConnectionUnix::Write(const void* data, size_t length)
{
    auto self = reinterpret_cast<BaseConnectionUnix*>(this);

    if (self->sock == -1) {
        return false;
    }

    ssize_t sentBytes = unix_write(self->sock, data, length);
    if (sentBytes < 0) {
        Close();
    }
    return sentBytes == (ssize_t)length;
}

bool BaseConnectionUnix::Read(void* data, size_t length)
{
    auto self = reinterpret_cast<BaseConnectionUnix*>(this);

    if (self->sock == -1) {
        return false;
    }

    int res = (int)unix_read(self->sock, data, length);
    if (res < 0) {
        if (res == EAGAIN) {
            return false;
        }
        Close();
    }
    else if (res == 0) {
        Close();
    }
    return res == (int)length;
}
