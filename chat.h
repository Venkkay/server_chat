/*
 * chat.h - Copyright (c) 2020 - Olivier Poncet
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
#ifndef __CHAT_H__
#define __CHAT_H__

#include <list>

// ---------------------------------------------------------------------------
// some declarations
// ---------------------------------------------------------------------------

using SockAddrIn = struct sockaddr_in;

// ---------------------------------------------------------------------------
// posix::signal_traits
// ---------------------------------------------------------------------------

namespace posix {

struct signal_traits
{
    static void handler(int signal);
};

}

// ---------------------------------------------------------------------------
// posix::sigset_traits
// ---------------------------------------------------------------------------

namespace posix {

struct sigset_traits
{
    using value_type = sigset_t;

    static void empty(value_type& sigset);

    static void fill(value_type& sigset);

    static void add(value_type& sigset, const int signum);

    static void del(value_type& sigset, const int signum);

    static void block(const value_type& sigset);

    static void setmask(const value_type& sigset);

    static void unblock(const value_type& sigset);
};

}

// ---------------------------------------------------------------------------
// posix::sigaction_traits
// ---------------------------------------------------------------------------

namespace posix {

struct sigaction_traits
{
    using value_type = struct sigaction;

    static void action(const int signum, const value_type* sigact, value_type* oldact);
};

}

// ---------------------------------------------------------------------------
// posix::sigset
// ---------------------------------------------------------------------------

namespace posix {

class sigset
{
public:
    sigset()
        : _sigset()
    {
        sigset_traits::empty(_sigset);
    }

    sigset_traits::value_type& operator*()
    {
        return _sigset;
    }

    const sigset_traits::value_type& operator*() const
    {
        return _sigset;
    }

    void empty()
    {
        sigset_traits::empty(_sigset);
    }

    void fill()
    {
        sigset_traits::fill(_sigset);
    }

    void add(const int signum)
    {
        sigset_traits::add(_sigset, signum);
    }

    void del(const int signum)
    {
        sigset_traits::del(_sigset, signum);
    }

    void block() const
    {
        sigset_traits::block(_sigset);
    }

    void setmask() const
    {
        sigset_traits::setmask(_sigset);
    }

    void unblock() const
    {
        sigset_traits::unblock(_sigset);
    }

private:
    sigset_traits::value_type _sigset;
};

}

// ---------------------------------------------------------------------------
// posix::sigaction
// ---------------------------------------------------------------------------

namespace posix {

class sigaction
{
public:
    sigaction(const int signum)
        : _signum(signum)
        , _sigaction()
        , _oldaction()
    {
    }

    sigaction_traits::value_type* operator->()
    {
        return &_sigaction;
    }

    const sigaction_traits::value_type* operator->() const
    {
        return &_sigaction;
    }

    sigaction_traits::value_type& operator*()
    {
        return _sigaction;
    }

    const sigaction_traits::value_type& operator*() const
    {
        return _sigaction;
    }

    void install()
    {
        sigaction_traits::action(_signum, &_sigaction, &_oldaction);
    }

    void restore()
    {
        sigaction_traits::action(_signum, &_oldaction, &_sigaction);
    }

private:
    const int                    _signum;
    sigaction_traits::value_type _sigaction;
    sigaction_traits::value_type _oldaction;
};

}

// ---------------------------------------------------------------------------
// SignalListener
// ---------------------------------------------------------------------------

class SignalListener
{
public:
    SignalListener() = default;

    virtual ~SignalListener() = default;

    virtual void onSigHgup();

    virtual void onSigIntr();

    virtual void onSigTerm();

    virtual void onSigPipe();

    virtual void onSigChld();

    virtual void onSigAlrm();

    virtual void onSigUsr1();

    virtual void onSigUsr2();
};

// ---------------------------------------------------------------------------
// SignalManager
// ---------------------------------------------------------------------------

class SignalManager
{
public:
    SignalManager(SignalListener& listener);

    virtual ~SignalManager() = default;

    virtual bool timedwait(const unsigned long timeout);

protected:
    SignalListener&  _listener;
    posix::sigset    _sigmask;
    posix::sigaction _sighgup;
    posix::sigaction _sigintr;
    posix::sigaction _sigterm;
    posix::sigaction _sigpipe;
    posix::sigaction _sigchld;
    posix::sigaction _sigalrm;
    posix::sigaction _sigusr1;
    posix::sigaction _sigusr2;
};

// ---------------------------------------------------------------------------
// EndPoint
// ---------------------------------------------------------------------------

class EndPoint
{
public:
    EndPoint()
        : EndPoint(INADDR_ANY, 0)
    {
    }

    EndPoint(const uint16_t port)
        : EndPoint(INADDR_ANY, port)
    {
    }

    EndPoint(const uint32_t addr, const uint16_t port)
        : _endpoint()
    {
        _endpoint.sin_family      = AF_INET;
        _endpoint.sin_addr.s_addr = htonl(addr);
        _endpoint.sin_port        = htons(port);
    }

    auto data() -> sockaddr*
    {
        return reinterpret_cast<sockaddr*>(&_endpoint);
    }

    auto data() const -> const sockaddr*
    {
        return reinterpret_cast<const sockaddr*>(&_endpoint);
    }

    auto size() const -> socklen_t
    {
        return sizeof(_endpoint);
    }

private:
    SockAddrIn _endpoint;
};

// ---------------------------------------------------------------------------
// Socket
// ---------------------------------------------------------------------------

class Socket
{
public:
    Socket();

    Socket(const int fd);

    virtual ~Socket();

    int fd() const
    {
        return _fd;
    }

    void reset(const int fd)
    {
        _fd = (close(), fd);
    }

    void create();

    void set_fd(int fd);

    void close();

    void bind(const uint32_t addr, const uint16_t port);

    void listen(const int backlog);

    int  accept();

    void send(const std::string&);

    void recv(std::string&);

    bool get_acceptconn() const;

    bool get_keepalive() const;

    void set_keepalive(const bool value) const;

    bool get_reuseaddr() const;

    void set_reuseaddr(const bool value) const;

    int  get_sndbuf() const;

    void set_sndbuf(const int value) const;

    int  get_rcvbuf() const;

    void set_rcvbuf(const int value) const;

protected:
    int _fd;
};

// ---------------------------------------------------------------------------
// ChatServer
// ---------------------------------------------------------------------------

class ChatServer final
    : protected SignalListener
{
public:
    ChatServer();

    virtual ~ChatServer() = default;

    void run(const uint32_t addr, const uint16_t port);

protected:
    virtual void onSigHgup() override;

    virtual void onSigIntr() override;

    virtual void onSigTerm() override;

    virtual void onSigPipe() override;

    virtual void onSigChld() override;

    virtual void onSigAlrm() override;

    virtual void onSigUsr1() override;

    virtual void onSigUsr2() override;

private:
    void cont();

    void quit();

    void sendMsgToClient(Socket client, std::string msg);

private:
    SignalManager _signal_manager;
    Socket        _server;
    std::list<Socket>       _clients;
    bool          _quit;
    std::vector<pollfd> _pollfds;
};

// ---------------------------------------------------------------------------
// End-Of-File
// ---------------------------------------------------------------------------

#endif /* __CHAT_H__ */
