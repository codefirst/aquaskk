/* -*- C++ -*-

   Copyright (c) 2008, Tomotaka SUWA All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   Neither the name of the organization nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
   COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef socketutil_h
#define socketutil_h

#include <cassert>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <algorithm>
#include <set>

#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#endif

namespace net {
namespace socket {
#ifdef _MSC_VER
    typedef int (__stdcall *initializer)(SOCKET, const sockaddr*, socklen_t);
    typedef int (__stdcall *converter)(SOCKET, sockaddr*, socklen_t*);
#else
    typedef int (*initializer)(int, const sockaddr*, socklen_t);
    typedef int (*converter)(int, sockaddr*, socklen_t*);
#endif

    // ======================================================================
    // closing functor
    // ======================================================================
    class closer {
        closer();
        closer(const closer&);
        closer& operator=(const closer&);

    public:
        closer(int fd) {
            if(fd != -1) {
#ifdef _MSC_VER
                closesocket(fd);
#else
                ::close(fd);
#endif
            }
        }

        operator int() const { return -1; }
    };

    // ======================================================================
    // built-in socket option functors
    // ======================================================================
    struct DEFAULT_option {
        int operator()(int) const { return 0; }
    };

    struct SO_REUSEADDR_option {
        int operator()(int fd) const {
            int optval = 1;
            return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
        }
    };

    // ======================================================================
    // address resolver(socket factory)
    // ======================================================================
    class address {
        addrinfo* result_;
        addrinfo* iter_;

        void clear() {
            if(!result_) return;

            freeaddrinfo(result_);
            result_ = iter_ = 0;
        }

        address(const address&);
        address& operator=(const address&);

    public:
        address() : result_(0), iter_(0) {}
        ~address() { clear(); }

        bool resolve(const char* node, const char* service, int flags, int family, int socktype, int protocol) {
            clear();

            addrinfo hints = {};

            hints.ai_flags = flags;
            hints.ai_family = family;
            hints.ai_socktype = socktype;
            hints.ai_protocol = protocol;

            if(getaddrinfo(node, service, &hints, &result_) == 0) {
                iter_ = result_;
            }

            return iter_ != 0;
        }

        operator const addrinfo*() const {
            return iter_;
        }

        void next() {
            if(iter_) iter_ = iter_->ai_next;
        }

        template <typename T>
        int get(const T& option, initializer init) {
            assert(iter_);

            int fd = static_cast<int>(::socket(iter_->ai_family, iter_->ai_socktype, iter_->ai_protocol));

            if(-1 == fd ||
               -1 == option(fd) ||
               -1 == init(fd, iter_->ai_addr, iter_->ai_addrlen)) {
                return closer(fd);
            }

            return fd;
        }
    };

    // ======================================================================
    // endpoint abstraction
    // ======================================================================
    class endpoint {
        std::string node_;
        std::string service_;

        const char* ptr(const std::string& str) const {
            if(!str.empty()) {
                return str.c_str();
            }

            return 0;
        }

    public:
        endpoint() {}

        endpoint(const char* node, const char* service) {
            if(node) node_ = node;
            if(service) service_ = service;
        }

        endpoint(const std::string& host, unsigned short port) {
            node_ = host;

            std::ostringstream buf;
            buf << port;
            service_ = buf.str();
        }

        void parse(const std::string& addr, const std::string& default_port) {
            std::string tmp(addr);
            std::replace(tmp.begin(), tmp.end(), ':', ' ');

            std::istringstream buf(tmp);

            service_ = default_port;

            buf >> node_ >> service_;
        }

        const std::string& node() const { return node_; }

        const std::string& service() const { return service_; }

        int connect(int family = AF_INET, int type = SOCK_STREAM, int protocol = 0, int flags = 0) const {
            return connect(family, type, protocol, flags, DEFAULT_option());
        }

        template <typename T>
        int connect(int family, int type, int protocol, int flags, const T& option) const {
            address addr;

            if(addr.resolve(ptr(node_), ptr(service_), flags, family, type, protocol)) {
                return addr.get(option, ::connect);
            }

            return -1;
        }

        int bind(int family, int type, int protocol = 0, int flags = 0) const {
            return bind(family, type, protocol, flags, SO_REUSEADDR_option());
        }

        template <typename T>
        int bind(int family, int type, int protocol, int flags, const T& option) const {
            address addr;

            if(addr.resolve(ptr(node_), ptr(service_), flags | AI_PASSIVE, family, type, protocol)) {
                return addr.get(option, ::bind);
            }

            return -1;
        }

        int listen(int backlog, int family = AF_INET, int type = SOCK_STREAM, int protocol = 0, int flags = 0) const {
            return listen(backlog, family, type, protocol, flags, SO_REUSEADDR_option());
        }

        template <typename T>
        int listen(int backlog, int family, int type, int protocol, int flags, const T& option) const {
            int fd = bind(family, type, protocol, flags, option);

            if(fd == -1 || ::listen(fd, backlog) == -1) {
                return closer(fd);
            }

            return fd;
        }
    };

    // ======================================================================
    // node name and service name
    // ======================================================================
    typedef std::pair<std::string, std::string> namepair;

    // ======================================================================
    // getnameinfo abstraction
    // ======================================================================
    class nameinfo {
        static namepair info(const sockaddr* saddr, int length, int option) {
            char host[NI_MAXHOST] = {};
            char service[NI_MAXSERV] = {};

            if(getnameinfo(saddr, length, host, sizeof(host), service, sizeof(service), option) == 0) {
                return namepair(host, service);
            }

            return namepair();
        }

        static namepair get(int fd, converter convert, int option) {
            sockaddr_storage storage = {};
            socklen_t length = sizeof(storage);
            namepair result;

            if(convert(fd, (sockaddr*)&storage, &length) == 0) {
                result = nameinfo::info((const sockaddr*)&storage, length, option);
            }

            return result;
        }

    public:
        static const namepair remote(int fd, int option = NI_NUMERICHOST | NI_NUMERICSERV) {
            return nameinfo::get(fd, getpeername, option);
        }

        static const namepair local(int fd, int option = NI_NUMERICHOST | NI_NUMERICSERV) {
            return nameinfo::get(fd, getsockname, option);
        }

        static const namepair addr(const addrinfo* info, int option = NI_NUMERICHOST | NI_NUMERICSERV) {
            return nameinfo::info(info->ai_addr, info->ai_addrlen, option);
        }
    };

    // ======================================================================
    // streambuf
    // ======================================================================
    class streambuf : public std::streambuf {
        int fd_;
        char* input_;
        char* output_;

        enum { BUFFER_SIZE = 8192 }; // TODO: should be a customize point

        virtual int_type overflow(int_type ch) {
            if(traits_type::eq_int_type(traits_type::eof(), ch)) {
                return traits_type::not_eof(ch);
            }

            // flush buffer
            if(sync() == -1) return traits_type::eof();

            return sputc(ch);
        }

        virtual int sync() {
            long remain = pptr() - pbase();
            int offset = 0;

            while(0 < remain) {
                ssize_t result = ::send(fd_, pbase() + offset, remain, 0);
                if(result == -1) return -1;

                remain -= result;
                offset += result;
            }

            setp(pbase(), epptr());

            return 0;
        }

        virtual int_type underflow() {
            setg(eback(), eback(), eback());

            // fill buffer
            ssize_t length = ::recv(fd_, gptr(), BUFFER_SIZE, 0);
            if(length == 0 || length == -1) return traits_type::eof();

            setg(eback(), gptr(), gptr() + length);

            return *gptr();
        }

    public:
        streambuf(int fd = -1) : fd_(fd) {
            // for recv
            input_ = new char[BUFFER_SIZE];
            setg(input_, input_, input_);

            // for send
            output_ = new char[BUFFER_SIZE];
            setp(output_, output_ + BUFFER_SIZE);
        }

        virtual ~streambuf() {
            delete[] output_;
            delete[] input_;
        }

        void attach(int fd) {
            fd_ = fd;

            setp(pbase(), epptr());
            setg(eback(), eback(), eback());
        }
    };

    // ======================================================================
    // tcpstream
    // ======================================================================
    class tcpstream : public std::iostream {
        int fd_;
        streambuf buf_;

        void assign(int fd) {
            buf_.attach(fd_ = fd);

            (fd_ != -1) ? clear() : setstate(std::ios::badbit);
        }

    public:
        tcpstream(int fd = -1) : std::iostream(0) {
            rdbuf(&buf_);
            assign(fd);
        }

        tcpstream(const std::string& host, unsigned short port, int family = AF_INET, int flags = 0) : std::iostream(0) {
            rdbuf(&buf_);
            open(host, port, family, flags);
        }

        tcpstream(const endpoint& endpoint, int family = AF_INET, int flags = 0) : std::iostream(0) {
            rdbuf(&buf_);
            open(endpoint, family, flags);
        }

        virtual ~tcpstream() {
            close();
        }

        void attach(int fd) {
            assign(fd);
        }

        void open(const std::string& host, unsigned short port, int family = AF_INET, int flags = 0) {
            open(endpoint(host, port), family, flags);
        }

        void open(const endpoint& endpoint, int family = AF_INET, int flags = 0) {
            assign(endpoint.connect(family, SOCK_STREAM, 0, flags));
        }

        void close() {
            flush();
            assign(closer(fd_));
        }

        int socket() const { return fd_; }
    };

    // ======================================================================
    // tcpserver
    // ======================================================================
    class tcpserver {
        int fd_;

        tcpserver(const tcpserver&);
        tcpserver& operator=(const tcpserver&);

        enum { BACKLOG_SIZE = 32 };

    public:
        tcpserver() {
            attach(-1);
        }

        tcpserver(unsigned short port, int backlog = BACKLOG_SIZE, int family = AF_INET, int flags = 0) {
            open(port, backlog, family, flags);
        }

        tcpserver(const endpoint& endpoint, int backlog = BACKLOG_SIZE, int family = AF_INET, int flags = 0) {
            open(endpoint, backlog, family, flags);
        }

        ~tcpserver() {
            close();
        }

        void attach(int fd) {
            fd_ = fd;
        }

        void open(unsigned short port, int backlog = BACKLOG_SIZE, int family = AF_INET, int flags = 0) {
            open(endpoint("", port), backlog, family, flags);
        }

        void open(const endpoint& endpoint, int backlog = BACKLOG_SIZE, int family = AF_INET, int flags = 0) {
            attach(endpoint.listen(backlog, family, SOCK_STREAM, 0, flags));
        }

        void close() {
            attach(closer(fd_));
        }

        int accept() const {
            return static_cast<int>(::accept(fd_, 0, 0));
        }

        int socket() const { return fd_; }

        operator void*() {
            return fd_ != -1 ? this : 0;
        }
    };

    // ======================================================================
    // fdset
    // ======================================================================
    class fdset {
        std::set<int> set_;
        fd_set impl_;

        void setup(int fd) {
            FD_SET(fd, &impl_);
        }

    public:
        void add(int fd) {
            set_.insert(fd);
        }

        void remove(int fd) {
            set_.erase(fd);
        }

        bool test(int fd) {
            return FD_ISSET(fd, &impl_);
        }

        int maxfd() {
            std::set<int>::iterator iter = std::max_element(set_.begin(), set_.end());
            if(iter != set_.end()) {
                return *iter;
            }
            return 0;
        }

        void reset() {
            FD_ZERO(&impl_);

            std::for_each(set_.begin(), set_.end(), std::bind1st(std::mem_fun(&fdset::setup), this));
        }

        operator fd_set*() {
            if(!set_.empty()) {
                return &impl_;
            }

            return 0;
        }
    };

    // ======================================================================
    // monitor
    // ======================================================================
    class monitor {
        fdset read_;
        fdset write_;
        fdset exception_;

        fdset& target(int type) {
            switch(type) {
            case WRITE:
                return write_;

            case EXCEPTION:
                return exception_;

            default:
                break;
            }

            return read_;
        }

        int nfds() {
            int r = read_.maxfd();
            int w = write_.maxfd();
            int e = exception_.maxfd();

            int max = (r < w) ? w : r;
            return ((max < e) ? e : max) + 1;
        }

        int select(timeval* tv = 0) {
            read_.reset();
            write_.reset();
            exception_.reset();

            return ::select(nfds(), read_, write_, exception_, tv);
        }

    public:
        enum monitor_types { READ, WRITE, EXCEPTION };

        void add(int fd, monitor_types type) {
            assert(fd != -1);
            target(type).add(fd);
        }

        void remove(int fd, monitor_types type) {
            target(type).remove(fd);
        }

        bool test(int fd, monitor_types type) {
            return target(type).test(fd);
        }

        int wait() {
            return select();
        }

        int wait(long seconds, int micro_seconds = 0) {
            timeval tv;

            tv.tv_sec = seconds;
            tv.tv_usec = micro_seconds;

            return select(&tv);
        }
    };
}
}

#ifdef _MSC_VER
namespace {
    class winsock2_environment {
        int result_;

        winsock2_environment(const winsock2_environment&);
        winsock2_environment& operator=(const winsock2_environment&);

    public:
        winsock2_environment() {
            WSAData data;
            result_ = WSAStartup(MAKEWORD(2, 2), &data);
        }

        ~winsock2_environment() {
            WSACleanup();
        }

        int startup_result() const { return result_; }
    };

    static winsock2_environment winsock2_environment_instance;
};
#endif

#endif
