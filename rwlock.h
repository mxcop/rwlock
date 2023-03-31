/**
 * @file rwlock.h
 * @author Max (mxcop)
 * @brief Read / Write lock with write priority.
 * @date 2023-03-31
 * 
 * Inspired by <https://github.com/Kl4rry/RwLock/blob/main/rwlock.h>
 */
#pragma once
#include <mutex>
#include <condition_variable>

/* Forward declare guards */
template <class T>
class WriteGuard;
template <class T>
class ReadGuard;

/**
 * @brief Read / Write lock allows one writer or any number of readers.
 * 
 * Implements pre-lock for write locks.
 * This means write locks will be given priority over read locks.
 * To ensure that writing isn't blocked forever by read locks.
 */
template <class T>
class RwLock {
    /* Access private vars from the guards */
    friend WriteGuard<T>;
    friend ReadGuard<T>;

    private:
        T data;
        std::mutex mtx;
        std::condition_variable cond_var;
        int rc;
        int wc;
        bool prelock;
    public:
        RwLock();
        ReadGuard<T> read();
        WriteGuard<T> write();
};

template <class T>
inline RwLock<T>::RwLock() { rc = wc = 0; prelock = false; }

/**
 * @brief Thread safe read guard for RwLock.
 */
template <class T>
class ReadGuard {
    /* Access private vars from the lock */
    friend RwLock<T>;

    public:
        ~ReadGuard();

        const T& data() {
            return lock.data;
        }

        const T* operator->() {
            return &lock.data;
        }
    private:
        ReadGuard(RwLock<T>& lock);
        RwLock<T>& lock;
};

template <class T>
inline ReadGuard<T>::ReadGuard(RwLock<T>& lock) : lock{lock} {}

/**
 * @brief Thread safe write guard for RwLock.
 */
template <class T>
class WriteGuard {
    /* Access private vars from the lock */
    friend RwLock<T>;

    public:
        ~WriteGuard();

        T& data() {
            return lock.data;
        }

        T* operator->() {
            return &lock.data;
        }

        T& operator=(const T& other) {
            if (&lock.data == &other) return lock.data;

            lock.data = other;
            return lock.data;
        }
    private:
        WriteGuard(RwLock<T>& lock);
        RwLock<T>& lock;
};

template <class T>
inline WriteGuard<T>::WriteGuard(RwLock<T>& lock) : lock{lock} {}

template <class T>
inline ReadGuard<T> RwLock<T>::read() {
    std::unique_lock<std::mutex> lock_guard(mtx);

    while(wc > 0 || prelock) {
        cond_var.wait(lock_guard);
    }

    ++rc;
    ReadGuard guard(*this);
    return guard;
}

template <class T>
inline WriteGuard<T> RwLock<T>::write() {
    std::unique_lock<std::mutex> lock_guard(mtx);

    prelock = true;
    while(rc > 0 || wc > 0) {
        cond_var.wait(lock_guard);
    }

    ++wc;
    WriteGuard<T> guard(*this);
    prelock = false;
    return guard;
}

template <class T>
inline ReadGuard<T>::~ReadGuard() {
    std::unique_lock<std::mutex> lock_guard(lock.mtx);
    --lock.rc;
    if(lock.rc < 1) {
        lock.cond_var.notify_one();
    }
}

template <class T>
inline WriteGuard<T>::~WriteGuard() {
    std::unique_lock<std::mutex> lock_guard(lock.mtx);
    --lock.wc;
    lock.cond_var.notify_all();
}
