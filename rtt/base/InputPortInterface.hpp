/***************************************************************************
  tag: Peter Soetens  Thu Oct 22 11:59:07 CEST 2009  InputPortInterface.hpp

                        InputPortInterface.hpp -  description
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


#ifndef ORO_INPUT_PORT_INTERFACE_HPP
#define ORO_INPUT_PORT_INTERFACE_HPP

#include <string>
#include "PortInterface.hpp"
#include "ChannelElement.hpp"
#include "../internal/rtt-internal-fwd.hpp"
#include "../internal/ConnectionManager.hpp"
#include "../Event.hpp"

namespace RTT
{ namespace base {


    /**
     * The base class of the InputPort. It contains the connection management code, which is
     * independent of the actual data being transmitted.
     */
    class RTT_API InputPortInterface : public PortInterface
    {
    public:
        typedef Event<void(PortInterface*)> NewDataOnPortEvent;

    protected:
        internal::ConnectionManager cmanager;
        ConnPolicy        default_policy;
        NewDataOnPortEvent* new_data_on_port_event;

    public:
        InputPortInterface(std::string const& name, ConnPolicy const& default_policy = ConnPolicy());
        ~InputPortInterface();

        /** Clears the connection. After call to read() will return false after
         * clear() has been called
         */
        void clear();

        ConnPolicy getDefaultPolicy() const;

        virtual bool addConnection(internal::ConnID* port_id, ChannelElementBase::shared_ptr channel_input, ConnPolicy const& policy = ConnPolicy() );

        /** Removes the input channel
         *
         * You should usually not use this directly. Use disconnect()
         * instead.
         */
        virtual void removeConnection(internal::ConnID* cid);

        /** Returns a DataSourceBase interface to read this port. The returned
         * data source is always the same object and will be destroyed when the
         * port is destroyed.
         */
        virtual DataSourceBase* getDataSource() = 0;

        /** Reads the port and updates the value hold by the given data source.
         * This is only valid for local ports.
         *
         * \a source has to be an assignable data source
         */
        virtual FlowStatus read(DataSourceBase::shared_ptr source);

        /** Removes any connection that either go to or come from this port */
        virtual void disconnect();

        /** Removes the channel that connects this port to \c port */
        virtual void disconnect(PortInterface& port);


        /** Returns true if this port is connected */
        virtual bool connected() const;

        /**
         * Call this to indicate that the connection leading to this port
         * is ready to use. The input port will check its channel elements
         * by sending an inputReady() message. If this succeeds, this
         * function returns true and the input port is ready to use (this->connected() == true).
         * If sending inputReady() returns failure, this method returns
         * false and the connection is aborted (this->connected() == false).
         */
        virtual bool channelReady(base::ChannelElementBase::shared_ptr channel);

        /** Returns the event object that gets emitted when new data is
         * available for this port. It gets deleted when the port is deleted.
         */
        NewDataOnPortEvent* getNewDataOnPortEvent();

        virtual bool connectTo(PortInterface& other, ConnPolicy const& policy);

        virtual bool connectTo(PortInterface& other);
};

}}

#endif
