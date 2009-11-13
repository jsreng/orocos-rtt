/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:26 CET 2004  DataObjectLocked.hpp

                        DataObjectLocked.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

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

#ifndef CORELIB_DATAOBJECT_LOCKED_HPP
#define CORELIB_DATAOBJECT_LOCKED_HPP


#include "../os/MutexLock.hpp"
#include "DataObjectInterface.hpp"

namespace RTT
{ namespace base {

    /**
     * @brief A class which provides locked/protected access to one typed element of data.
     *
     * It allows multiple read/write requests using a single lock. This is the in any case
     * threadsafe implementation, and can be blocking in situations where you do not want
     * that.
     * @ingroup Ports
     */
    template<class T>
    class DataObjectLocked
        : public DataObjectInterface<T>
    {
        mutable os::Mutex lock;

        /**
         * One element of Data.
         */
        T data;
    public:
        /**
         * Construct a DataObjectLocked by name.
         *
         * @param _name The name of this DataObject.
         */
        DataObjectLocked( const T& initial_value = T() )
            : data(initial_value) {}

        /**
         * The type of the data.
         */
        typedef T DataType;

        virtual void Get( DataType& pull ) const { os::MutexLock locker(lock); pull = data; }

        virtual DataType Get() const { DataType cache;  Get(cache); return cache; }

        virtual void Set( const DataType& push ) { os::MutexLock locker(lock); data = push; }

        virtual void data_sample( const DataType& sample ) {
            Set(sample);
        }

        virtual DataObjectLocked<DataType>* clone() const {
            return new DataObjectLocked<DataType>();
        }

        virtual DataObjectLocked<DataType>* copy( std::map<const DataSourceBase*, DataSourceBase*>&  ) const {
            return const_cast<DataObjectLocked<DataType>*>(this);
        }
    };
}}

#endif

