/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  ConnectionC.cxx

                        ConnectionC.cxx -  description
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


#include "../interface/EventService.hpp"
#include "ConnectionC.hpp"
#include "FactoryExceptions.hpp"
#include "EventProcessor.hpp"
#include "../Logger.hpp"
#include <vector>
#include <boost/bind.hpp>

namespace RTT
{

    using namespace detail;
    using namespace boost;

    class ConnectionC::D
    {
    public:
        const EventService* mgcf;
        std::string mname;
        shared_ptr<EventCallBack> syn_ecb;
        shared_ptr<EventCallBack> asyn_ecb;
        EventProcessor::AsynStorageType ms_type;
        EventProcessor* mep;
        Handle h;

        /**
         * This method creates connections and stores the Handle objects
         * in \a h.
         */
        void checkAndCreate() {
            Logger::In in("ConnectionC");
            if (syn_ecb ) {
                Logger::log() << Logger::Info << "Creating Syn connection to "+ mname +"."<<Logger::endl;
                // syn_ecb is never deleted !
                h = mgcf->setupSyn(mname, bind(&EventCallBack::callback, syn_ecb), syn_ecb->args());
                if (!h)
                    Logger::log() << Logger::Error << "Creating Syn connection to "+ mname +" failed."<<Logger::endl;
                syn_ecb.reset();
            }
            if (asyn_ecb) {
                Logger::log() << Logger::Info << "Creating Asyn connection to "+ mname +"."<<Logger::endl;
                // asyn_ecb is never deleted !
                h = mgcf->setupAsyn(mname, bind(&EventCallBack::callback, asyn_ecb), asyn_ecb->args(), mep, ms_type);
                if (!h)
                    Logger::log() << Logger::Error << "Creating ASyn connection to "+ mname +" failed."<<Logger::endl;
                asyn_ecb.reset();
            }
        }

        void callback(EventCallBack* ecb) {
            if (syn_ecb || asyn_ecb) {
                Logger::log() << Logger::Error << "Ignoring added Synchronous 'callback': already present."<<Logger::endl;
                delete ecb;
                return;
            }
            syn_ecb.reset( ecb );
        }
        void callback(EventCallBack* ecb, EventProcessor* ep, EventProcessor::AsynStorageType s_type) {
            if (syn_ecb || asyn_ecb) {
                Logger::log() << Logger::Error << "Ignoring added Asynchronous 'callback': already present."<<Logger::endl;
                delete ecb;
                return;
            }
            asyn_ecb.reset( ecb ); mep = ep; ms_type = s_type;
        }

        D( const EventService* gcf, const std::string& name)
            : mgcf(gcf), mname(name), syn_ecb(), asyn_ecb(), mep(0), h()
        {
        }

        D(const D& other)
            : mgcf( other.mgcf), mname(other.mname),
              syn_ecb( other.syn_ecb ), asyn_ecb( other.asyn_ecb ),
              mep( other.mep ), h( other.h )
        {
            // We steal the connection info from other.
            //other.syn_ecb = 0;
            //other.asyn_ecb = 0;
        }

        ~D()
        {
            //delete syn_ecb;
            //delete asyn_ecb;
        }

        /**
         * When handle is invoked, all the created connections
         * are returned.
         */
        Handle handle() {
            this->checkAndCreate();
            return h;
        }

    };

    ConnectionC::ConnectionC()
        : d(0)
    {}

    ConnectionC::ConnectionC(const EventService* gcf, const std::string& name)
        : d( gcf ? new D( gcf, name ) : 0 )
    {
    }

    ConnectionC::ConnectionC(const ConnectionC& other)
        : d( other.d ? new D(*other.d) : 0 )
    {
    }

    ConnectionC& ConnectionC::operator=(const ConnectionC& other)
    {
        if ( &other == this )
            return *this;
        delete d;
        d = ( other.d ? new D(*other.d) : 0 );
        return *this;
    }

    ConnectionC::~ConnectionC()
    {
        delete d;
    }

    bool ConnectionC::ready() const
    {
        return d && d->h;
    }

    ConnectionC& ConnectionC::mcallback( EventCallBack* ecb)
    {
        Logger::In in("ConnectionC");
        if (d ) {
            d->callback( ecb );
        } else {
            Logger::log() << Logger::Warning << "Ignoring added 'callback' on empty Connection."<<Logger::endl;
            delete ecb; // created by us.
        }
        return *this;
    }

    ConnectionC& ConnectionC::mcallback( EventCallBack* ecb, EventProcessor* ep, EventProcessor::AsynStorageType s_type)
    {
        Logger::In in("ConnectionC");
        if (d) {
            d->callback( ecb, ep, s_type );
        } else {
            Logger::log() << Logger::Warning << "Ignoring added 'callback' on empty Connection."<<Logger::endl;
            delete ecb; // created by us.
        }
        return *this;
    }

    Handle ConnectionC::handle() {
        Logger::In in("ConnectionC");

        Handle h;
        if (d)
            h = d->handle();

        // now check if we actually got a valid handle
        // if nothing new was added, 'h' still has the previous added connections.
        if ( !h ) {
            Logger::log() << Logger::Warning << "Used handle() on empty Connection. Returned invalid handle to Event "+ (d ? d->mname : "") +"."<<Logger::endl;
        }
        return h;
    }
}
