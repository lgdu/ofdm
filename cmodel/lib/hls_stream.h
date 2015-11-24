/*
   Copyright (C) 2013 XILINX, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef X_HLS_STREAM_SIM_H
#define X_HLS_STREAM_SIM_H

/*
 * This file contains a C++ model of hls::stream.
 * It defines C simulation model.
 */
#ifndef __cplusplus
#error C++ is required to include this header file

#else

#ifdef HLS_STREAM_THREAD_SAFE
//////////////////////////////////////////////
// Mutex class
//////////////////////////////////////////////
#if defined WIN32 || defined _WIN32
#include <windows.h>
#include <process.h>
typedef ::CRITICAL_SECTION mutex_handle;
typedef ::HANDLE cond_var_handle;
#else
#include <pthread.h>
#include <sys/types.h>
#include <stddef.h>
typedef pthread_mutex_t mutex_handle;
typedef pthread_cond_t cond_var_handle;
#endif

class ap_mutex
{
    friend class ap_cond_var;

protected:
    mutex_handle m_Mutex;

public:
    // Create a mutex
    ap_mutex() {
#if defined WIN32 || defined _WIN32
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0403)
        // for better performance.
        ::InitializeCriticalSectionAndSpinCount( &m_Mutex, 4000 );
#else
        ::InitializeCriticalSection( &m_Mutex );
#endif
#else
        pthread_mutex_init(&m_Mutex, NULL);
#endif
    }
    ~ap_mutex() {
#if defined WIN32 || defined _WIN32
        ::DeleteCriticalSection( &m_Mutex );
#else
        pthread_mutex_destroy(&m_Mutex);
#endif
    }

    // Lock the ap_mutex.
    inline bool lock()
    {
#if defined WIN32 || defined _WIN32
        ::EnterCriticalSection( &m_Mutex );
#else 
        if ( pthread_mutex_lock(&m_Mutex) != 0 ) return false;
#endif
        return true;
    }

    // Unlock the ap_mutex
    inline void unlock()
    {
#if defined WIN32 || defined _WIN32
        ::LeaveCriticalSection( &m_Mutex );
#else
        pthread_mutex_unlock(&m_Mutex);
#endif
    }

    inline bool trylock()
    {
#if defined WIN32 || defined _WIN32
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0400)
        return ::TryEnterCriticalSection( &m_Mutex ) != 0;
#else
        return false;
#endif
#else
        return pthread_mutex_trylock(&m_Mutex) != 0;
#endif
    }

  protected:
    inline mutex_handle* getMutex() { return &m_Mutex; }

}; // End of ap_Mutex class

class ap_cond_var
{
protected:
    cond_var_handle m_Cond_Var;

public:
    // Create a Condition Variable
    ap_cond_var() {
#if defined WIN32 || defined _WIN32
        m_Cond_Var = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
        pthread_cond_init(&m_Cond_Var, NULL);
#endif
    }
    ~ap_cond_var() {
#if !defined WIN32 || !defined _WIN32
        pthread_cond_destroy(&m_Cond_Var);
#endif
    }

    // restarts one of the threads that are waiting on the condition variable cond.
    inline void signal()
    {
#if defined WIN32 || defined _WIN32
        ::SetEvent(m_Cond_Var);
#else 
        pthread_cond_signal(&m_Cond_Var);
#endif
    }

    // unlocks the mutex and waits for the condition variable cond to be signaled.
    inline void wait(ap_mutex& _m_mutex)
    {
#if defined WIN32 || defined _WIN32
        ::WaitForSingleObject(m_Cond_Var, INFINITE);
#else
        pthread_cond_wait(&m_Cond_Var, _m_mutex.getMutex());
#endif
    }

}; // End of ap_cond_var class
#endif


//////////////////////////////////////////////
// C level simulation models for hls::stream
//////////////////////////////////////////////
#include <queue>
#include <iostream>
#include <typeinfo>
#include <string>
#include <sstream>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif

namespace hls {

template<typename __STREAM_T__>
class stream
{
  protected:
    std::string _name;
    std::deque<__STREAM_T__> _data; // container for the elements
#ifdef HLS_STREAM_THREAD_SAFE
    ap_mutex _m_mutex;
    ap_cond_var _m_cond_var;
#endif    

  public:
    /// Constructors
    // Keep consistent with the synthesis model's constructors
    stream() {
        static unsigned _counter = 1;
        std::stringstream ss;
#ifndef _MSC_VER
        _name = abi::__cxa_demangle(typeid(*this).name(), 0, 0, 0);
#else
		_name = typeid(*this).name();
#endif

        ss << _counter++;
        _name += "." + ss.str();
    }

    stream(const std::string name) {
    // default constructor,
    // capacity set to predefined maximum
        _name = name;
    }

  /// Make copy constructor and assignment operator private
  private:
    stream(const stream< __STREAM_T__ >& chn):
        _name(chn._name), _data(chn._data) {
    }

    stream& operator = (const stream< __STREAM_T__ >& chn) {
        _name = chn._name;
        _data = chn._data;
        return *this;
    }

  public:
    /// Overload >> and << operators to implement read() and write()
    void operator >> (__STREAM_T__& rdata) {
        read(rdata);
    }

    void operator << (const __STREAM_T__& wdata) {
        write(wdata);
    }


  public:
    /// Destructor
    /// Check status of the queue
    virtual ~stream() {
        if (!_data.empty())
        {
            std::cout << "WARNING: Hls::stream '" 
                      << _name 
                      << "' contains leftover data,"
                      << " which may result in RTL simulation hanging."
                      << std::endl;
        }
    }

    /// Status of the queue
#ifdef HLS_STREAM_THREAD_SAFE
    bool empty() {
#else
    bool empty() const {
#endif
#ifdef HLS_STREAM_THREAD_SAFE
        _m_mutex.lock();
#endif    
        bool t = _data.empty();
#ifdef HLS_STREAM_THREAD_SAFE
        _m_mutex.unlock();
#endif    
        return t;
    }
    bool full() const { return false; }

    /// Blocking read
    void read(__STREAM_T__& head) {
        head = read();
    }

#ifdef HLS_STREAM_THREAD_SAFE
    __STREAM_T__ read() {

        _m_mutex.lock();

        while (_data.empty()) {
            _m_cond_var.wait(_m_mutex);
        }

        __STREAM_T__ elem;
        elem = _data.front();
        _data.pop_front();
        _m_mutex.unlock();

        return elem;
    }
#else
    __STREAM_T__ read() {

        __STREAM_T__ elem;
        if (_data.empty()) {
            std::cout << "WARNING: Hls::stream '"
                      << _name 
                      << "' is read while empty,"
                      << " which may result in RTL simulation hanging."
                      << std::endl;
            elem = __STREAM_T__();
        } else {
            elem = _data.front();
            _data.pop_front();
        }

        return elem;
    }
#endif

    /// Blocking write
    void write(const __STREAM_T__& tail) { 
#ifdef HLS_STREAM_THREAD_SAFE
        _m_mutex.lock();
#endif  
        _data.push_back(tail);
#ifdef HLS_STREAM_THREAD_SAFE
        _m_mutex.unlock();
        _m_cond_var.signal();
#endif    
    }

    /// Nonblocking read
    bool read_nb(__STREAM_T__& head) {
#ifdef HLS_STREAM_THREAD_SAFE
        _m_mutex.lock();
#endif    
        bool is_empty = _data.empty();
        if (is_empty) {
            head = __STREAM_T__();
        } else {
            __STREAM_T__ elem(_data.front());
            _data.pop_front();
            head = elem;
        }
#ifdef HLS_STREAM_THREAD_SAFE
        _m_mutex.unlock();
#endif    
        return !is_empty;
    }

    /// Nonblocking write
    bool write_nb(const __STREAM_T__& tail) {
        bool is_full = full();
        write(tail);
        return !is_full;
    }

    /// Fifo size
    size_t size() {
        return _data.size();
    }
};

} // namespace hls

#endif // __cplusplus
#endif  // X_HLS_STREAM_SIM_H
