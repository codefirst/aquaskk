/* -*- C++ -*-
 *
 * pthreadutil.h - C++ pthread wrapper.
 *
 *   Copyright (c) 2007-2008 Tomotaka SUWA, All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef pthreadutil_h
#define pthreadutil_h

#include <pthread.h>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <deque>
#include <iostream>
#include <functional>

namespace pthread {
    class err {
        const char* msg_;

    public:
        err(const char* msg) : msg_(msg) {}

#ifdef DEBUG
        void operator()() {
            std::cerr << "ERR: " << msg_
                      << ": " << std::strerror(errno)
                      << ": errno=" << errno << std::endl;
        }
#else
        void operator()() {}
#endif
    };
    
    class mutex {
	friend class lock;
	friend class condition;

	pthread_mutex_t handle_;

	mutex(const mutex&);
	mutex& operator=(const mutex&);

    public:
	mutex() {
	    if(pthread_mutex_init(&handle_, 0) != 0) {
                err("pthread_mutex_init");
            }
	};

	~mutex() {
            if(pthread_mutex_destroy(&handle_) != 0) {
                err("pthread_mutex_destroy");
            }
	}
    };

    class suspend_cancel {
	void set(int state) {
	    int tmp;
	    if(pthread_setcancelstate(state, &tmp) != 0) {
                err("pthread_setcancelstate");
            }
	}

	suspend_cancel(const suspend_cancel&);
	suspend_cancel& operator=(const suspend_cancel&);

    public:
	suspend_cancel() {
	    set(PTHREAD_CANCEL_DISABLE);
	}

	~suspend_cancel() {
	    set(PTHREAD_CANCEL_ENABLE);
	}
    };

    class lock {
	suspend_cancel shield_;
	mutex& target_;

	lock();
	lock(const lock&);
	lock& operator=(const lock&);

    public:
	lock(mutex& target) : target_(target) {
	    if(pthread_mutex_lock(&target_.handle_) != 0) {
                err("pthread_mutex_lock");
            }
	}

	~lock() {
	    if(pthread_mutex_unlock(&target_.handle_) != 0) {
                err("pthread_mutex_unlock");
            }
	}
    };

    class condition {
	mutex mutex_;
	pthread_cond_t handle_;

	condition(const condition&);
	condition& operator=(const condition&);

	void trylock() {
	    assert(EBUSY == pthread_mutex_trylock(&mutex_.handle_) &&
		   "*** You MUST lock the pthread::condition object to avoid race conditions. ***");
	}

    public:
	condition() {
	    pthread_cond_init(&handle_, 0);
	}

	~condition() {
	    pthread_cond_destroy(&handle_);
	}

	operator mutex&() {
	    return mutex_;
	}

	void signal() {
	    trylock();

	    if(pthread_cond_signal(&handle_) != 0) {
                err("pthread_cond_signal");
            }
	}

	void broadcast() {
	    trylock();

	    if(pthread_cond_broadcast(&handle_) != 0) {
                err("pthread_cond_broadcast");
            }
	}

	bool wait() {
	    trylock();

	    if(pthread_cond_wait(&handle_, &mutex_.handle_) != 0) {
                err("pthread_cond_wait");
                return false;
            }

            return true;
	}

	bool wait(long second, long nano_second = 0) {
	    trylock();

	    timespec abstime;
	    abstime.tv_sec = std::time(0) + second;
	    abstime.tv_nsec = nano_second;

	    if(pthread_cond_timedwait(&handle_, &mutex_.handle_, &abstime) != 0) {
                err("pthread_cond_timedwait");
                return false;
            }

            return true;
	}
    };

    class task {
    public:
	virtual ~task() {}
	virtual bool run() = 0;
    };

    class timer {
	task* task_;
	long interval_;
	long startup_delay_;
	pthread_t thread_;

	static void* handler(void* param) {
	    timer* self = reinterpret_cast<timer*>(param);

	    self->run();

	    return 0;
	}

	void run() {
	    wait(startup_delay_);

	    while(task_->run()) {
		wait(interval_);
	    }
	}

	void wait(long second) {
	    if(!second) return;

	    timespec ts;
	    ts.tv_sec = second;
	    ts.tv_nsec = 0;

	    nanosleep(&ts, 0);
	}

	timer();
	timer(const timer&);
	timer& operator=(const timer&);

    public:
	timer(task* task, long interval, long startup_delay = 0)
	    : task_(task), interval_(interval), startup_delay_(startup_delay) {
	    if(pthread_create(&thread_, 0, timer::handler, this) != 0) {
		thread_ = 0;
	    }
	}

	~timer() {
	    if(thread_ && pthread_cancel(thread_) == 0) {
		pthread_join(thread_, 0);
	    }
	}
    };

    class pool {
        condition queue_;       // for tasks_ and should_terminate_
        condition worker_;      // for idle_threads_
        std::deque<pthread_t> pool_;
        std::deque<task*> tasks_;
        bool should_terminate_;
        unsigned long idle_threads_;

        static void* handler(void* param) {
            pool* self = reinterpret_cast<pool*>(param);

            self->run();

            return 0;
        }

        void run() {
            while(task* task = dequeue()) {
                acquire();

                task->run();
                delete task;

                release();
            }
        }

        task* dequeue() {
            while(1) {
                lock scope(queue_);

                if(tasks_.empty() && should_terminate_) return 0;

                if(!tasks_.empty() || queue_.wait()) {
                    if(tasks_.empty()) {
                        if(should_terminate_) return 0;
                        continue;
                    }

                    task* next = tasks_.front();
                    tasks_.pop_front();

                    return next;
                }
            }
        }

        void acquire() {
            lock scope(worker_);
            -- idle_threads_;
        }

        void release() {
            lock scope(worker_);
            ++ idle_threads_;

            if(idle_threads_ == pool_.size()) {
                worker_.signal();
            }
        }

        pool(const pool&);
        pool& operator=(const pool&);

    public:
        pool(unsigned size = 4) : should_terminate_(false) {
            lock scope(worker_);

            for(unsigned i = 0; i < size; ++ i) {
                pthread_t id;
                if(pthread_create(&id, 0, pool::handler, this) == 0) {
                    pool_.push_back(id);
                } else {
                    err("pthread_create");
                }
            }

            idle_threads_ = pool_.size();
        }

        ~pool() {
            /* set terminate flag */ {
                lock scope(queue_);
                should_terminate_ = true;
            }

            while(1) {
                lock scope(worker_);

                if(idle_threads_ == pool_.size() || worker_.wait()) {
                    lock scope(queue_);

                    if(tasks_.empty()) {
                        queue_.broadcast();
                        break;
                    }
                }
            }

            for(auto thread : pool_) {
                pthread_join(thread, 0);
            }
        }

        bool enqueue(task* newly_allocated_task) {
            lock scope(queue_);

            if(!should_terminate_) {
                tasks_.push_back(newly_allocated_task);
                queue_.signal();
                return true;
            }

            return false;
        }
    };
};

#endif
