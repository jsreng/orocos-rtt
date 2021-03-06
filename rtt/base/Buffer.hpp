/***************************************************************************
  tag: Peter Soetens  Thu Oct 22 11:59:07 CEST 2009  Buffer.hpp

                        Buffer.hpp -  description
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



#ifndef ORO_BUFFER_HPP
#define ORO_BUFFER_HPP

#include "../rtt-config.h"

/**
 * @file We can't use typedefs since C++ doesn't allow it for
 * templated classes without specifying all the template
 * parameters.
 */
#if defined(OROBLD_OS_NO_ASM)
#include "BufferLocked.hpp"
#else
#include "BufferLocked.hpp"
#include "BufferLockFree.hpp"
#endif

namespace RTT
{ namespace base {

    /**
     * This object represents the default thread-safe buffer implementation used
     * by Orocos objects.
     */
    template< class T>
    class Buffer
#if defined(OROBLD_OS_NO_ASM)
        : public BufferLocked<T>
#else
        : public BufferLockFree<T>
#endif
    {
    public:
        typedef typename BufferInterface<T>::reference_t reference_t;
        typedef typename BufferInterface<T>::param_t param_t;
        typedef typename BufferInterface<T>::size_type size_type;
        typedef T value_t;

    public:
        /**
         * Create a fifo queue of fixed size.
         */
        Buffer( int qsize, const T& initial_value = T())
#if defined(OROBLD_OS_NO_ASM)
            : BufferLocked<Ty>(qsize, initial_value)
#else
            : BufferLockFree<T>(qsize, initial_value)
#endif
        {}
    };
}}


#endif /* ORO_BUFFER_HPP */
