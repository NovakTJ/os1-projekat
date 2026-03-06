#include "../h/semaphore.hpp"
#include "../h/tcb.hpp"

_sem::_sem(unsigned init) : value((int)init), blockedQueue() {
    // TODO: implement
}

_sem::~_sem() {
    // TODO: implement
}

int _sem::open(_sem** handle, unsigned init) {
    // TODO: implement
    return 0;
}

int _sem::close(_sem* handle) {
    // TODO: implement
    return 0;
}

int _sem::wait() {
    // TODO: implement
    return 0;
}

int _sem::signal() {
    // TODO: implement
    return 0;
}
