/*
 * chat.cc - Copyright (c) 2020 - Olivier Poncet
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <csignal>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <iostream>
#include <stdexcept>
#include "chat.h"

#include <algorithm>

// ---------------------------------------------------------------------------
// posix::signal_traits
// ---------------------------------------------------------------------------

namespace posix {

void signal_traits::handler(int signum)
{
    std::cout << "signal" << ' ' << signum << ' ' << "received" << std::endl;
}

}

// ---------------------------------------------------------------------------
// posix::sigset_traits
// ---------------------------------------------------------------------------

namespace posix {

void sigset_traits::empty(value_type& sigset)
{
    const int rc = ::sigemptyset(&sigset);

    if(rc != 0) {
        throw std::runtime_error("sigemptyset() has failed");
    }
}

void sigset_traits::fill(value_type& sigset)
{
    const int rc = ::sigfillset(&sigset);

    if(rc != 0) {
        throw std::runtime_error("sigfillset() has failed");
    }
}

void sigset_traits::add(value_type& sigset, const int signum)
{
    const int rc = ::sigaddset(&sigset, signum);

    if(rc != 0) {
        throw std::runtime_error("sigaddset() has failed");
    }
}

void sigset_traits::del(value_type& sigset, const int signum)
{
    const int rc = ::sigdelset(&sigset, signum);

    if(rc != 0) {
        throw std::runtime_error("sigdelset() has failed");
    }
}

void sigset_traits::block(const value_type& sigset)
{
    const int rc = ::pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    if(rc != 0) {
        throw std::runtime_error("sigprocmask() has failed");
    }
}

void sigset_traits::setmask(const value_type& sigset)
{
    const int rc = ::pthread_sigmask(SIG_SETMASK, &sigset, nullptr);

    if(rc != 0) {
        throw std::runtime_error("sigprocmask() has failed");
    }
}

void sigset_traits::unblock(const value_type& sigset)
{
    const int rc = ::pthread_sigmask(SIG_UNBLOCK, &sigset, nullptr);

    if(rc != 0) {
        throw std::runtime_error("sigprocmask() has failed");
    }
}

}

// ---------------------------------------------------------------------------
// posix::sigaction_traits
// ---------------------------------------------------------------------------

namespace posix {

void sigaction_traits::action(const int signum, const value_type* sigact, value_type* oldact)
{
    const int rc = ::sigaction(signum, sigact, oldact);

    if(rc != 0) {
        throw std::runtime_error("sigaction() has failed");
    }
}

}

// ---------------------------------------------------------------------------
// posix::sigset
// ---------------------------------------------------------------------------

namespace posix {

}

// ---------------------------------------------------------------------------
// posix::sigaction
// ---------------------------------------------------------------------------

namespace posix {

}

// ---------------------------------------------------------------------------
// SignalListener
// ---------------------------------------------------------------------------

void SignalListener::onSigHgup()
{
}

void SignalListener::onSigIntr()
{
}

void SignalListener::onSigTerm()
{
}

void SignalListener::onSigPipe()
{
}

void SignalListener::onSigChld()
{
}

void SignalListener::onSigAlrm()
{
}

void SignalListener::onSigUsr1()
{
}

void SignalListener::onSigUsr2()
{
}

// ---------------------------------------------------------------------------
// SignalManager
// ---------------------------------------------------------------------------

SignalManager::SignalManager(SignalListener& listener)
    : _listener(listener)
    , _sigmask()
    , _sighgup(SIGHUP)
    , _sigintr(SIGINT)
    , _sigterm(SIGTERM)
    , _sigpipe(SIGPIPE)
    , _sigchld(SIGCHLD)
    , _sigalrm(SIGALRM)
    , _sigusr1(SIGUSR1)
    , _sigusr2(SIGUSR2)
{
    auto prepare = [](posix::sigset& sigset) -> void
    {
        sigset.add(SIGHUP);
        sigset.add(SIGINT);
        sigset.add(SIGTERM);
        sigset.add(SIGPIPE);
        sigset.add(SIGCHLD);
        sigset.add(SIGALRM);
        sigset.add(SIGUSR1);
        sigset.add(SIGUSR2);
        sigset.block();
    };

    auto install = [](posix::sigaction& sigaction, posix::sigset& sigset) -> void
    {
        sigaction->sa_handler = &posix::signal_traits::handler;
        sigaction->sa_flags   = SA_RESTART;
        sigaction->sa_mask    = *sigset;
        sigaction.install();
    };

    prepare(_sigmask);
    install(_sighgup, _sigmask);
    install(_sigintr, _sigmask);
    install(_sigterm, _sigmask);
    install(_sigpipe, _sigmask);
    install(_sigchld, _sigmask);
    install(_sigalrm, _sigmask);
    install(_sigusr1, _sigmask);
    install(_sigusr2, _sigmask);
}

bool SignalManager::timedwait(const unsigned long timeout)
{
    struct timespec ts;
    ts.tv_sec  = (timeout / 1000UL) * 1UL;
    ts.tv_nsec = (timeout % 1000UL) * 1000UL;

    const int rc = ::sigtimedwait(&(*_sigmask), nullptr, &ts);
    if(rc < 0) {
        if(errno == EAGAIN) {
            return false;
        }
        if(errno == EINTR) {
            return false;
        }
        throw std::runtime_error("sigtimedwait() has failed");
    }
    switch(rc) {
        case SIGHUP:
            _listener.onSigHgup();
            break;
        case SIGINT:
            _listener.onSigIntr();
            break;
        case SIGTERM:
            _listener.onSigTerm();
            break;
        case SIGPIPE:
            _listener.onSigPipe();
            break;
        case SIGCHLD:
            _listener.onSigChld();
            break;
        case SIGALRM:
            _listener.onSigAlrm();
            break;
        case SIGUSR1:
            _listener.onSigUsr1();
            break;
        case SIGUSR2:
            _listener.onSigUsr2();
            break;
        default:
            break;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Socket
// ---------------------------------------------------------------------------

Socket::Socket()
    : Socket(-1)
{
}

Socket::Socket(const int fd)
    : _fd(fd)
{
}

Socket::~Socket()
{
    try {
        close();
    }
    catch(const std::exception& e) {
        static_cast<void>(e);
    }
}

void Socket::create()
{
    if(_fd < 0) {
        const int rc = ::socket(AF_INET, SOCK_STREAM, 0);
        if(rc >= 0) {
            _fd = rc;
        }
        else {
            throw std::runtime_error("socket() has failed");
        }
    }
}

void Socket::set_fd(const int fd)
{
    _fd = fd;
}

void Socket::close()
{
    if(_fd >= 0) {
        const int rc = ::close(_fd);
        if(rc == 0) {
            _fd = -1;
        }
        else {
            throw std::runtime_error("close has failed");
        }
    }
}

void Socket::bind(const uint32_t addr, const uint16_t port)
{
    const EndPoint endpoint(addr, port);

    const int rc = ::bind(_fd, endpoint.data(), endpoint.size());
    if(rc < 0) {
        throw std::runtime_error("bind() has failed");
    }
}

void Socket::listen(const int backlog)
{
    const int rc = ::listen(_fd, backlog);
    if(rc < 0) {
        throw std::runtime_error("listen() has failed");
    }
}

int Socket::accept()
{
    SockAddrIn addr = {};
    socklen_t  size = sizeof(addr);
    const int rc = ::accept(_fd, reinterpret_cast<sockaddr*>(&addr), &size);
    if(rc < 0) {
        throw std::runtime_error("accept() has failed");
    }
    return rc;
}

void Socket::send(const std::string& string)
{
    const ssize_t rc = ::send(_fd, string.data(), string.size(), 0);
    if(rc < 0) {
        std::cerr << "Error sending to client " << _fd << std::endl;
        close();
        throw std::runtime_error("send() has failed");
    }
}

void Socket::recv(std::string& string)
{
    char data[1024];

    const ssize_t rc = ::recv(_fd, data, sizeof(data), 0);
    if(rc < 0) {
        throw std::runtime_error("recv() has failed");
    }
    else if(rc == 0) {
        close();
    }
    else {
        std::string(data, rc).swap(string);
    }
}

bool Socket::get_acceptconn() const
{
    int       option_val = 0;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_ACCEPTCONN, &option_val, &option_len);
    if(rc < 0) {
        throw std::runtime_error("getsockopt() has failed");
    }
    return option_val;
}

bool Socket::get_keepalive() const
{
    int       option_val = 0;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &option_val, &option_len);
    if(rc < 0) {
        throw std::runtime_error("getsockopt() has failed");
    }
    return option_val;
}

void Socket::set_keepalive(const bool value) const
{
    int       option_val = value;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::setsockopt(_fd, SOL_SOCKET, SO_KEEPALIVE, &option_val, option_len);
    if(rc < 0) {
        throw std::runtime_error("setsockopt() has failed");
    }
}

bool Socket::get_reuseaddr() const
{
    int       option_val = 0;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &option_val, &option_len);
    if(rc < 0) {
        throw std::runtime_error("getsockopt() has failed");
    }
    return option_val;
}

void Socket::set_reuseaddr(const bool value) const
{
    int       option_val = value;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &option_val, option_len);
    if(rc < 0) {
        throw std::runtime_error("setsockopt() has failed");
    }
}

int Socket::get_sndbuf() const
{
    int       option_val = 0;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &option_val, &option_len);
    if(rc < 0) {
        throw std::runtime_error("getsockopt() has failed");
    }
    return option_val;
}

void Socket::set_sndbuf(const int value) const
{
    int       option_val = value;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::setsockopt(_fd, SOL_SOCKET, SO_SNDBUF, &option_val, option_len);
    if(rc < 0) {
        throw std::runtime_error("setsockopt() has failed");
    }
}

int Socket::get_rcvbuf() const
{
    int       option_val = 0;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::getsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &option_val, &option_len);
    if(rc < 0) {
        throw std::runtime_error("getsockopt() has failed");
    }
    return option_val;
}

void Socket::set_rcvbuf(const int value) const
{
    int       option_val = value;
    socklen_t option_len = sizeof(option_val);
    const int rc = ::setsockopt(_fd, SOL_SOCKET, SO_RCVBUF, &option_val, option_len);
    if(rc < 0) {
        throw std::runtime_error("setsockopt() has failed");
    }
}

// ---------------------------------------------------------------------------
// ChatServer
// ---------------------------------------------------------------------------

ChatServer::ChatServer()
    : SignalListener()
    , _signal_manager(*this)
    , _server()
    , _clients()
    , _quit(false)
{
}

void ChatServer::run(const uint32_t addr, const uint16_t port)
{
    std::cout << "ChatServer::run()" << std::endl;
    //std::s
    _server.create();
    _server.set_reuseaddr(true);
    _server.bind(addr, port);
    _server.listen(5);

    //std::vector<pollfd> pollfds;
    _pollfds.push_back({_server.fd(), POLLIN, 0});
    _pollfds.push_back({STDIN_FILENO, POLLIN, 0});

    while (!_quit) {

        if (_signal_manager.timedwait(1000)) {
            continue;
        }

        int poll_count = poll(_pollfds.data(), _pollfds.size(), -1);
        if (poll_count < 0) {
            perror("Poll failed");
            break;
        }

        for (size_t i = 0; i < _pollfds.size(); ++i) {
            if (_pollfds[i].revents & POLLIN) {
                // Nouveau client en attente de connexion
                if (_pollfds[i].fd == _server.fd()) {
                    int client_fd = _server.accept();
                    _clients.emplace_back(client_fd);

                    std::cout << "New client connected: " << client_fd << std::endl;

                    // Ajouter le client à la liste
                    _pollfds.push_back({client_fd, POLLIN, 0});
                }
                else if (_pollfds[i].fd == STDIN_FILENO) {
                    std::string input;
                    if (!std::getline(std::cin, input)) {
                        std::cout << "Error reading from stdin" << std::endl;
                        continue;
                    }
                    if (input == "quit") {
                        quit();
                    }
                    else if (!input.empty()){
                        input.append("\r\n");
                        for (auto& client : _clients) {
                            sendMsgToClient(client, input);
                        }
                    }
                }
                // Données reçues d'un client
                else {
                    char buffer[1024];
                    int bytes_read = recv(_pollfds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytes_read <= 0) {
                        // Client déconnecté
                        std::cout << "Client disconnected: " << _pollfds[i].fd << std::endl;
                        close(_pollfds[i].fd);
                        _pollfds.erase(_pollfds.begin() + i);
                        for (auto it = _clients.begin(); it != _clients.end(); ++it) {
                            if (it->fd() == _pollfds[i].fd) {
                                _clients.erase(it);
                                break;
                            }
                        }
                        --i;
                    } else {
                        // Traiter et répondre au client
                        buffer[bytes_read-1] = '\0';
                        std::cout << "Message from client " << _pollfds[i].fd << ": " << buffer << std::endl;
                    }
                }
            }
        }
    }

}


void ChatServer::cont()
{
}

void ChatServer::quit()
{
    _server.close();
    for (Socket _client : _clients) {
        _client.close();
    }
    _quit = true;
}

void ChatServer::sendMsgToClient(Socket& client, std::string msg) {
    try {
        client.send(msg);
    } catch (const std::exception& e) {
        printf("%s", e.what());
        _pollfds.erase(std::remove_if(_pollfds.begin(), _pollfds.end(),[&client](const pollfd& pfd) {
                return pfd.fd == client.fd();
            }), _pollfds.end());
        _clients.remove_if([&client](const Socket& c) {
            return c.fd() == client.fd();
        });
    }
}

void ChatServer::onSigHgup()
{
    std::cout << "SIGHGUP" << std::endl;
    quit();
}

void ChatServer::onSigIntr()
{
    std::cout << "SIGINTR" << std::endl;
    quit();
}

void ChatServer::onSigTerm()
{
    std::cout << "SIGTERM" << std::endl;
    quit();
}

void ChatServer::onSigPipe()
{
    std::cout << "SIGPIPE" << std::endl;
    quit();
}

void ChatServer::onSigChld()
{
    std::cout << "SIGCHLD" << std::endl;
    quit();
}

void ChatServer::onSigAlrm()
{
    std::cout << "SIGALRM" << std::endl;
    cont();
}

void ChatServer::onSigUsr1()
{
    std::cout << "SIGUSR1" << std::endl;
    cont();
}

void ChatServer::onSigUsr2()
{
    std::cout << "SIGUSR2" << std::endl;
    cont();
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    const uint32_t addr = INADDR_ANY;
    const uint16_t port = 1976;

    try {
        ChatServer chat_server;

        chat_server.run(addr, port);
    }
    catch(const std::exception& e) {
        const char* what(e.what());
        std::cerr << "error: " << what << std::endl;
        return EXIT_FAILURE;
    }
    catch(...) {
        const char* what("unhandled exception");
        std::cerr << "error: " << what << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------
