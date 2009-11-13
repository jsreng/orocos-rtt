/***************************************************************************
  tag: Peter Soetens  Thu Oct 22 11:59:07 CEST 2009  CorbaDispatcher.hpp

                        CorbaDispatcher.hpp -  description
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


#ifndef ORO_CORBA_DISPATCHER_HPP
#define ORO_CORBA_DISPATCHER_HPP

#include "../../os/MutexLock.hpp"
#include "../../Activity.hpp"
#include "../../base/ChannelElementBase.hpp"
#include "../../Logger.hpp"
#include "../../internal/List.hpp"
#include "DataFlowI.h"
#include "../../interface/DataFlowInterface.hpp"

namespace RTT {
    namespace corba {
        /**
         * This object sends over data flow messages
         * from local buffers to a remote channel element.
         */
        class CorbaDispatcher : public Activity
        {
            typedef std::map<interface::DataFlowInterface*,CorbaDispatcher*> DispatchMap;
            static DispatchMap DispatchI;

            typedef internal::List<base::ChannelElementBase::shared_ptr> RCList;
            RCList RClist;

            bool do_exit;

            static os::Mutex mlock;

            CorbaDispatcher( const std::string& name)
            : Activity(ORO_SCHED_RT, os::LowestPriority, 0.0, 0, name),
              RClist(20,2),
              do_exit(false)
              {}

        public:
            /**
             * Create a new dispatcher for a given data flow interface.
             * This method will only lock and allocate when a new dispatcher must be created,
             * otherwise, the access is lock-free and real-time.
             * One dispatcher per \a iface is created.
             * @param iface The interface to dispatch data flow messages for.
             * @return
             */
            static CorbaDispatcher* Instance(interface::DataFlowInterface* iface) {
                DispatchMap::iterator result = DispatchI.find(iface);
                if ( result == DispatchI.end() ) {
                    os::MutexLock lock(mlock);
                    // re-try to find (avoid race):
                    result = DispatchI.find(iface);
                    if ( result != DispatchI.end() )
                        return result->second;
                    // *really* not found, let's create it.
                    std::string name;
                    if ( iface == 0 || iface->getParent() == 0)
                        name = "Global";
                    else
                        name = iface->getParent()->getName();
                    name += ".CorbaDispatch";
                    DispatchI[iface] = new CorbaDispatcher( name );
                    DispatchI[iface]->start();
                    return DispatchI[iface];
                }
                return result->second;
            }

            void dispatchChannel( base::ChannelElementBase::shared_ptr chan ) {
                RClist.append( chan );
                this->trigger();
            }

            void cancelChannel( base::ChannelElementBase::shared_ptr chan ) {
                RClist.erase( chan );
            }

            bool initialize() {
                log(Info) <<"Started " << this->getName() << "." <<endlog();
                do_exit = false;
                return true;
            }

            void loop() {
                while ( !RClist.empty() && !do_exit) {
                    base::ChannelElementBase::shared_ptr chan = RClist.front();
                    CRemoteChannelElement_i* rbase = dynamic_cast<CRemoteChannelElement_i*>(chan.get());
                    if (rbase)
                        rbase->transferSamples();
                    RClist.erase( chan );
                }
            }

            bool breakLoop() {
                do_exit = true;
                return true;
            }
        };
    }
}
#endif
