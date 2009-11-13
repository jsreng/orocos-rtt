/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:39 CET 2006  BufferLocked.hpp

                        BufferLocked.hpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/





#ifndef ORO_CORELIB_BUFFER_LOCKED_HPP
#define ORO_CORELIB_BUFFER_LOCKED_HPP

#include "../os/Mutex.hpp"
#include "../os/MutexLock.hpp"
#include "BufferInterface.hpp"
#include "BufferPolicy.hpp"
#include <deque>

namespace RTT
{ namespace base {


    /**
     * Implements a very simple blocking threadsafe buffer, using mutexes (locks).
     *
     * @see BufferLockFree
     * @ingroup Ports
     */
    template<class T, class ReadPolicy = NonBlockingPolicy, class WritePolicy = NonBlockingPolicy>
    class BufferLocked
        :public BufferInterface<T>
    {
    public:

        typedef typename ReadInterface<T>::reference_t reference_t;
        typedef typename WriteInterface<T>::param_t param_t;
        typedef typename BufferInterface<T>::size_type size_type;
        typedef T value_t;

        /**
         * Create a buffer of size \a size.
         */
        BufferLocked( size_type size, const T& initial_value = T() )
            : cap(size), buf(), write_policy(size), read_policy(0)
        {
            data_sample(initial_value);
        }

        virtual void data_sample( const T& sample )
        {
            buf.resize(cap, sample);
            buf.resize(0);
        }

        /**
         * Destructor
         */
        ~BufferLocked() {}

        bool Push( param_t item )
        {
            write_policy.pop();
            os::MutexLock locker(lock);
            if (cap == (size_type)buf.size() ) {
                write_policy.push();
                return false;
            }
            buf.push_back( item );
            read_policy.push();
            return true;
        }

        size_type Push(const std::vector<T>& items)
        {
            write_policy.pop( items.size() );
            os::MutexLock locker(lock);
            typename std::vector<T>::const_iterator itl( items.begin() );
            while ( ((size_type)buf.size() != cap) && (itl != items.end()) ) {
                buf.push_back( *itl );
                ++itl;
                read_policy.push();
            }
            write_policy.push( itl - items.begin() );
            return (itl - items.begin());

        }
        bool Pop( reference_t item )
        {
            read_policy.pop();
            os::MutexLock locker(lock);
            if ( buf.empty() ) {
                read_policy.push();
                return false;
            }
            item = buf.front();
            buf.pop_front();
            write_policy.push();
            return true;
        }

        size_type Pop(std::vector<T>& items )
        {
            os::MutexLock locker(lock);
            int quant = 0;
            while ( !buf.empty() ) {
                items.push_back( buf.front() );
                buf.pop_front();
                ++quant;
                read_policy.pop();
                write_policy.push();
            }
            return quant;
        }

        value_t front() const
        {
            os::MutexLock locker(lock);
            value_t item = value_t();
            if ( !buf.empty() )
                item = buf.front();
            return item;
        }

        size_type capacity() const {
            os::MutexLock locker(lock);
            return cap;
        }

        size_type size() const {
            os::MutexLock locker(lock);
            return buf.size();
        }

        void clear() {
            os::MutexLock locker(lock);
            buf.clear();
        }

        bool empty() const {
            os::MutexLock locker(lock);
            return buf.empty();
        }

        bool full() const {
            os::MutexLock locker(lock);
            return (size_type)buf.size() ==  cap;
        }
    private:
        size_type cap;
        std::deque<T> buf;
        mutable os::Mutex lock;
        WritePolicy write_policy;
        ReadPolicy read_policy;
    };
}}

#endif // BUFFERSIMPLE_HPP
