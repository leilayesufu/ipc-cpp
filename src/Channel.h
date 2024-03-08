#ifndef CHANNEL_H
#define CHANNEL_H
/// Hyper simple pipe-based IPC channel
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <stdexcept>
#include <tuple>
#include <memory>

namespace ipc {
template<typename T>
class Sender {
public:
    Sender(): m_fd(-1) {}
    explicit Sender(int fd): m_fd(fd) {}

    Sender(const Sender<T>& other) = delete;
    Sender(const Sender<T>&& other) {
        m_fd = other.m_fd;
        other.m_fd = -1;
    }

    void send(T elem) {
        send(&elem);
    }

    void send(T* elem) {
        if ( write(m_fd, elem, sizeof(T)) == -1 )
            throw std::runtime_error(std::string("send error: ") + strerror(errno));
    }

    std::unique_ptr<Sender<T>> clone() {
        int clone = dup(m_fd);
        if ( clone == -1 )
            throw std::runtime_error(std::string("dup error: ") + strerror(errno));

        return std::unique_ptr<Sender<T>>(new Sender<T>(clone));
    }

    ~Sender() {
        if ( m_fd != -1 )
            close(m_fd);
    }
private:
    int m_fd;
};

template<typename T>
class Receiver {
public:
    Receiver(): m_fd(-1) {}
    explicit Receiver(int fd): m_fd(fd), m_non_blocking(false) {}
    explicit Receiver(int fd, bool non_blocking): m_fd(fd), m_non_blocking(non_blocking) {}

    Receiver(const Receiver<T>& other) = delete;
    Receiver(const Receiver<T>&& other) {
        m_fd = other.m_fd;
        m_non_blocking = other.m_non_blocking;
        other.m_fd = -1;
    }

    std::unique_ptr<Receiver<T>> clone() {
        int clone = dup(m_fd);
        if ( clone == -1 )
            throw std::runtime_error(std::string("dup error: ") + strerror(errno));

        return std::unique_ptr<Receiver<T>>(new Receiver<T>(clone, m_non_blocking));
    }

    void set_non_blocking(bool non_blocking = true) {
        if ( m_non_blocking == non_blocking )
            return;

        int flags = fcntl(m_fd, F_GETFL, 0);

        if ( non_blocking )
            flags |= O_NONBLOCK;
        else
            flags &= ~O_NONBLOCK;

        if ( fcntl(m_fd, F_SETFL, flags) == -1 )
            throw std::runtime_error(std::string("fnctl error: ") + strerror(errno));

        m_non_blocking = non_blocking;
    }

    T recv() {
        T ret;
        recv(&ret);

        return ret;
    }

    void recv(T* ret) {
        if ( m_non_blocking )
            throw std::runtime_error("blocking recv on non-blocking receiver");

        if ( read(m_fd, ret, sizeof(T)) != sizeof(T) )
            throw std::runtime_error(std::string("recv error: ") + strerror(errno));
    }

    bool try_recv(T* ret) {
        if ( ! m_non_blocking )
            throw std::runtime_error("non-blocking recv on blocking receiver");

        if ( read(m_fd, ret, sizeof(T)) != sizeof(T) ) {
            if ( errno == EAGAIN )
                return false;
            throw std::runtime_error(std::string("recv error: ") + strerror(errno));
        }

        return true;
    }

    ~Receiver() {
        if ( m_fd != -1 )
            close(m_fd);
    }
private:
    int m_fd;
    bool m_non_blocking;
};

template<typename T>
std::pair<std::unique_ptr<Sender<T>>, std::unique_ptr<Receiver<T>>> channel() {
    int fds[2];

    if ( pipe(fds) != 0 )
        throw std::runtime_error(std::string("channel error: ") + strerror(errno));

    return std::make_pair(std::unique_ptr<Sender<T>>(new Sender<T>(fds[1])),
                          std::unique_ptr<Receiver<T>>(new Receiver<T>(fds[0])));
}

} // namespace ipc

#endif
