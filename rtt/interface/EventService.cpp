/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  EventService.cxx

                        EventService.cxx -  description
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


#include "EventService.hpp"
#include "../internal/mystd.hpp"

#include "../ExecutionEngine.hpp"
#include "../internal/CompletionProcessor.hpp"
#include "../internal/ConnectionC.hpp"



namespace RTT {
    using namespace detail;
    using namespace boost;

    EventService::EventService( ExecutionEngine* ee ) : eeproc(ee), eproc( 0 ) {}
    EventService::EventService( EventProcessor* ep ) : eeproc(0), eproc( ep ) {} // ep may be null.

    EventProcessor* EventService::getEventProcessor() {
        return eeproc ? eeproc->events()
            : (eproc ? eproc : CompletionProcessor::Instance());
    }

    void EventService::setEventProcessor(EventProcessor* ep) {
        eproc = ep;
    }

    bool EventService::hasEvent(const std::string& ename) const
    {
        return mevents.count(ename) == 1;
    }

    std::vector<std::string> EventService::getEvents() const
    {
        return keys( mevents );
    }

    int EventService::arity(const std::string& name) const
    {
        return this->getArity(name);
    }

    bool EventService::removeEvent( const std::string& ename ) {
        if ( mevents.count(ename) == 0 )
            return false;
        // first erase the hook.
        for (Hooks::iterator it = mhooks.begin(); it !=mhooks.end(); ++it ) {
            if ( it->first == ename ) {
                delete it->second;
                mhooks.erase( it );
                break;
            }
        }
        // next erase the emittor.
        mevents.erase( mevents.find( ename ) );
        // next erase the factory, if any:
        this->remove( ename );
        return true;
    }

    void EventService::clear() {
        for (Hooks::iterator it = mhooks.begin(); it !=mhooks.end(); ++it )
            delete it->second;
        OperationFactory< ActionInterface*>::clear();
        mevents.clear();
    }


    EventService::~EventService() {
        clear();
    }

    EventC EventService::setupEmit(const std::string& ename) const {
        if ( !this->hasMember(ename) ) {
            log(Error) << "Can not initialize EventC for '"<<ename<<"': no such Event."<<endlog();
            return EventC(); // empty handle.
        }
        return EventC(this, ename);
    }

    ConnectionC EventService::setupConnection(const std::string& ename) const {
        if ( mhooks.count(ename) != 1 ) {
            log(Error) << "Can not initialize ConnectionC for '"<<ename<<"': no such Event."<<endlog();
            return ConnectionC(); // empty handle.
        }
        return ConnectionC(this, ename );
    }

    Handle EventService::setupSyn(const std::string& ename,
                                  boost::function<void(void)> func,
                                  std::vector<DataSourceBase::shared_ptr> args ) const {
        if ( mhooks.count(ename) != 1 ) {
            log(Error) << "Can not create connection to '"<<ename<<"': no such Event."<<endlog();
            return Handle(); // empty handle.
        }
        detail::EventHookBase* ehi = mhooks.find(ename)->second->produce( args );

        // ehi is stored _inside_ the connection object !
        return ehi->setupSyn( func );
    }

    Handle EventService::setupAsyn(const std::string& ename,
                                   boost::function<void(void)> afunc,
                                   const std::vector<DataSourceBase::shared_ptr>& args,
                                   EventProcessor* ep /* = 0 */,
                                   EventProcessor::AsynStorageType s_type) const {
        if ( mhooks.count(ename) != 1 ) {
            log(Error) << "Can not create connection to '"<<ename<<"': no such Event."<<endlog();
            return Handle(); // empty handle.
        }
        detail::EventHookBase* ehi = mhooks.find(ename)->second->produce( args );

        if (ep == 0)
            ep = CompletionProcessor::Instance();
        // ehi is stored _inside_ the connection object !
        return ehi->setupAsyn( afunc, ep, s_type );
    }

    ActionInterface* EventService::getEvent(const std::string& ename,const std::vector<DataSourceBase::shared_ptr>& args) const
    {
        return this->produce(ename, args);
    }


}
