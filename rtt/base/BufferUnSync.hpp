/***************************************************************************
  tag: Peter Soetens  Thu Oct 22 11:59:07 CEST 2009  BufferUnSync.hpp

                        BufferUnSync.hpp -  description
                           -------------------
    begin                : Thu October 22 2009
    copyright            : (C) 2009 Peter Soetens
    email                : peter@thesourcworks.com

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



#ifndef ORO_CORELIB_BUFFER_UNSYNC_HPP
#define ORO_CORELIB_BUFFER_UNSYNC_HPP

#include "BufferInterface.hpp"
#include <deque>

namespace RTT
{ namespace base {


    /**
     * Implements a \b not threadsafe buffer. Only use when no more than one
     * thread accesses this buffer at a time.
     *
     * @see BufferLockFree, BufferUnSync
     * @ingroup Ports
     */
    template<class T>
    class BufferUnSync
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
        BufferUnSync( size_type size, const T& initial_value = T() )
            : cap(size), buf()
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
        ~BufferUnSync() {}

        bool Push( param_t item )
        {
            if (cap == (size_type)buf.size() ) {
                return false;
            }
            buf.push_back( item );
            return true;
        }

        size_type Push(const std::vector<T>& items)
        {
            typename std::vector<T>::const_iterator itl( items.begin() );
            while ( ((size_type)buf.size() != cap) && (itl != items.end()) ) {
                buf.push_back( *itl );
                ++itl;
            }
            return (itl - items.begin());
        }
        bool Pop( reference_t item )
        {
            if ( buf.empty() ) {
                return false;
            }
            item = buf.front();
            buf.pop_front();
            return true;
        }

        size_type Pop(std::vector<T>& items )
        {
            int quant = 0;
            while ( !buf.empty() ) {
                items.push_back( buf.front() );
                buf.pop_front();
                ++quant;
            }
            return quant;
        }

        value_t front() const
        {
            value_t item = value_t();
            if ( !buf.empty() )
                item = buf.front();
            return item;
        }

        size_type capacity() const {
            return cap;
        }

        size_type size() const {
            return buf.size();
        }

        void clear() {
            buf.clear();
        }

        bool empty() const {
            return buf.empty();
        }

        bool full() const {
            return (size_type)buf.size() ==  cap;
        }
    private:
        size_type cap;
        std::deque<T> buf;
    };
}}

#endif // BUFFERSIMPLE_HPP
