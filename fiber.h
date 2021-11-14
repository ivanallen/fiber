#pragma once
#define _XOPEN_SOURCE

#include <atomic>
#include <iostream>
#include <memory>
#include <functional>
#include <ucontext.h>

class Fiber;

class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    enum State {
        INIT,
        RUNNING,
        TERMINATE,
    };
    typedef std::shared_ptr<Fiber> Ref;
    typedef void (*Func)();
    Fiber(std::function<void()> fn) : _fn(std::move(fn)) {
        _state = INIT;
        _fid = _fiber_count.fetch_add(1);
        std::cout << "Fiber():" << _fid << " p:" << this << std::endl;
        getcontext(&_ucp);
        _ucp.uc_stack.ss_sp = _stack;
        _ucp.uc_stack.ss_size = 1024*1024;
        _ucp.uc_link = &_oucp;
        makecontext(&_ucp, (Func)Fiber::run_this, 1, this);
    }

    Fiber() {
        _state = INIT;
        _fid = _fiber_count.fetch_add(1);
        std::cout << "Fiber():" << _fid << std::endl;
        getcontext(&_ucp);
    }

    ~Fiber() {
        _fiber_count--;
        std::cout << "~Fiber():" << _fid << std::endl;
    }

    uint64_t id() const {
        return _fid;
    }

    State state() const {
        return _state;
    }

    bool terminated() const {
        return _state == TERMINATE;
    }

    bool running() const {
        return _state == RUNNING;
    }

    // main fiber -> this fiber
    void resume() {
        if (_state == TERMINATE) return;
        _state = RUNNING;
        _parent = current;
        current = shared_from_this();
        swapcontext(&_oucp, &_ucp);
        current = _parent;
    }

    static void yield() {
        swapcontext(&current->_ucp, &current->_oucp);
    }

    static thread_local Fiber::Ref current;
private:
    static void run_this(void* arg) {
        Fiber* fiber = static_cast<Fiber*>(arg);
        fiber->_fn();
        current = fiber->_parent;
        fiber->_state = TERMINATE;
    }
private:
    static std::atomic<uint64_t> _fiber_count;
    uint64_t _fid = 0;
    State _state = INIT;
    std::function<void()> _fn;
    ucontext_t _ucp;
    ucontext_t _oucp;
    Ref _parent;
    char* _stack[1024*1024];
};

thread_local Fiber::Ref Fiber::current;
std::atomic<uint64_t> Fiber::_fiber_count = {0};
