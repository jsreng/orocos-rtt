/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:09:49 CET 2006  ControlTaskServer.cxx

                        ControlTaskServer.cxx -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@fmtc.be

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



#include "ControlTaskServer.hpp"
#include "ControlTaskProxy.hpp"
#include "corba.h"
#ifdef CORBA_IS_TAO
#include "ControlTaskS.h"
#include <orbsvcs/CosNamingC.h>
// ACE Specific, for printing exceptions.
#include <ace/SString.h>
#include "tao/TimeBaseC.h"
#include "tao/Messaging/Messaging.h"
#include "tao/Messaging/Messaging_RT_PolicyC.h"
#else
#include <omniORB4/Naming.hh>
#endif
#include "ControlTaskC.h"
#include "ControlTaskI.h"
#include "POAUtility.h"
#include <iostream>
#include <fstream>

#include "../../os/threads.hpp"
#include "../../Activity.hpp"

namespace RTT
{namespace corba
{
    using namespace std;

    std::map<TaskContext*, ControlTaskServer*> ControlTaskServer::servers;

    base::ActivityInterface* ControlTaskServer::orbrunner = 0;

  ControlTaskServer::~ControlTaskServer()
  {
    Logger::In in("~ControlTaskServer()");
    servers.erase(mtaskcontext);

    PortableServer::ObjectId_var oid = mpoa->servant_to_id(mtask_i.in());
    mpoa->deactivate_object(oid);

    if (muse_naming) {
        try {
            CORBA::Object_var rootObj = orb->resolve_initial_references("NameService");
            CosNaming::NamingContext_var rootNC = CosNaming::NamingContext::_narrow(rootObj.in());

            if (CORBA::is_nil( rootNC.in() ) ) {
                log(Warning) << "CControlTask '"<< mtaskcontext->getName() << "' could not find CORBA Naming Service."<<endlog();
            } else {
                // Nameserver found...
                CosNaming::Name name;
                name.length(2);
                name[0].id = CORBA::string_dup("ControlTasks");
                name[1].id = CORBA::string_dup( mtaskcontext->getName().c_str() );
                try {
                    rootNC->unbind(name);
                    log(Info) << "Successfully removed CControlTask '"<<mtaskcontext->getName()<<"' from CORBA Naming Service."<<endlog();
                }
                catch( ... ) {
                    log(Warning) << "CControlTask '"<< mtaskcontext->getName() << "' unbinding failed."<<endlog();
                }
            }
        } catch (...) {
            log(Warning) << "CControlTask '"<< mtaskcontext->getName() << "' unbinding failed from CORBA Naming Service."<<endlog();
        }
    }
  }




    ControlTaskServer::ControlTaskServer(TaskContext* taskc, bool use_naming, bool require_name_service)
      : mtaskcontext(taskc), muse_naming(use_naming)
    {
        Logger::In in("ControlTaskServer()");
        servers[taskc] = this;
        try {
            // Each server has its own POA.
            // The server's objects have their own poa as well.
            CORBA::Object_var poa_object =
                orb->resolve_initial_references ("RootPOA");
            mpoa = PortableServer::POA::_narrow(poa_object);
            PortableServer::POAManager_var poa_manager =
                mpoa->the_POAManager ();

            //poa = POAUtility::create_basic_POA( poa, poa_manager, taskc->getName().c_str(), 0, 1);
            //            poa_manager->activate ();

            // TODO : Use a better suited POA than create_basic_POA, use the 'session' or so type
            // But watch out: we need implicit activation, our you will get exceptions upon ->_this()
            // The POA for the Server's objects:
//             PortableServer::POA_var objpoa = POAUtility::create_basic_POA(poa,
//                                                               poa_manager,
//                                                               std::string(taskc->getName() + "OBJPOA").c_str(),
//                                                               0, 0); // Not persistent, allow implicit.

            // The servant : TODO : cleanup servant in destructor !
            Orocos_CControlTask_i* serv;
            mtask_i = serv = new Orocos_CControlTask_i( taskc, mpoa );
            mtask   = serv->activate_this();

            if ( use_naming ) {
                CORBA::Object_var rootObj;
                CosNaming::NamingContext_var rootNC;
                try {
                    rootObj = orb->resolve_initial_references("NameService");
                    rootNC = CosNaming::NamingContext::_narrow(rootObj);
                } catch (...) {}
            
                if (CORBA::is_nil( rootNC ) ) {
                    std::string  err("CControlTask '" + taskc->getName() + "' could not find CORBA Naming Service.");
                    if (require_name_service) {
                        log(Error) << err << endlog();
                        throw IllegalServer(err);
                    }
                    else
                    {
                        log(Warning) << err << endlog();
                        log() <<"Writing IOR to 'std::cerr' and file '" << taskc->getName() <<".ior'"<<endlog();

                        // this part only publishes the IOR to a file.
                        CORBA::String_var ior = orb->object_to_string( mtask.in() );
                        std::cerr << ior.in() <<std::endl;
                        {
                            // write to a file as well.
                            std::string iorname( taskc->getName());
                            iorname += ".ior";
                            std::ofstream file_ior( iorname.c_str() );
                            file_ior << ior.in() <<std::endl;
                        }
                        return;
                    }
                }
                log(Info) << "CControlTask '"<< taskc->getName() << "' found CORBA Naming Service."<<endlog();
                // Nameserver found...
                CosNaming::Name name;
                name.length(1);
                name[0].id = CORBA::string_dup("ControlTasks");
                CosNaming::NamingContext_var controlNC;
                try {
                    controlNC = rootNC->bind_new_context(name);
                }
                catch( CosNaming::NamingContext::AlreadyBound&) {
                    log(Debug) << "NamingContext 'ControlTasks' already bound to CORBA Naming Service."<<endlog();
                    // NOP.
                }

                name.length(2);
                name[1].id = CORBA::string_dup( taskc->getName().c_str() );
                try {
                    rootNC->bind(name, mtask );
                    log(Info) << "Successfully added CControlTask '"<<taskc->getName()<<"' to CORBA Naming Service."<<endlog();
                }
                catch( CosNaming::NamingContext::AlreadyBound&) {
                    log(Warning) << "CControlTask '"<< taskc->getName() << "' already bound to CORBA Naming Service."<<endlog();
                    log() <<"Trying to rebind...";
                    try {
                        rootNC->rebind(name, mtask);
                    } catch( ... ) {
                        log() << " failed!"<<endlog();
                        return;
                    }
                    log() << " done. New CControlTask bound to Naming Service."<<endlog();
                }
            } // use_naming
            else {
                log(Info) <<"CControlTask '"<< taskc->getName() << "' is not using the CORBA Naming Service."<<endlog();
                log() <<"Writing IOR to 'std::cerr' and file '" << taskc->getName() <<".ior'"<<endlog();

                // this part only publishes the IOR to a file.
                CORBA::String_var ior = orb->object_to_string( mtask.in() );
                std::cerr << ior.in() <<std::endl;
                {
                    // write to a file as well.
                    std::string iorname( taskc->getName());
                    iorname += ".ior";
                    std::ofstream file_ior( iorname.c_str() );
                    file_ior << ior.in() <<std::endl;
                }
                return;
            }
        }
        catch (CORBA::Exception &e) {
            log(Error) << "CORBA exception raised!" << endlog();
            log() << CORBA_EXCEPTION_INFO(e) << endlog();
        }

    }

    void ControlTaskServer::CleanupServers() {
        if ( !CORBA::is_nil(orb) ) {
            log(Info) << "Cleaning up ControlTaskServers..."<<endlog();
            ServerMap::iterator it = servers.begin();
            while ( !servers.empty() ){
                delete servers.begin()->second;
            }
            log() << "Cleanup done."<<endlog();
        }
    }

    void ControlTaskServer::ShutdownOrb(bool wait_for_completion)
    {
        Logger::In in("ShutdownOrb");
        DoShutdownOrb(wait_for_completion);
    }

    void ControlTaskServer::DoShutdownOrb(bool wait_for_completion)
    {
        if ( CORBA::is_nil(orb) ) {
            log(Error) << "Orb Shutdown...failed! Orb is nil." << endlog();
            return;
        }

        try {
            log(Info) << "Orb Shutdown...";
            if (wait_for_completion)
                log(Info)<<"waiting..."<<endlog();
            orb->shutdown( wait_for_completion );
            log(Info) << "done." << endlog();
        }
        catch (CORBA::Exception &e) {
            log(Error) << "Orb Shutdown...failed! CORBA exception raised." << endlog();
            log() << CORBA_EXCEPTION_INFO(e) << endlog();
            return;
        }
    }


    void ControlTaskServer::RunOrb()
    {
        if ( CORBA::is_nil(orb) ) {
            log(Error) << "RunOrb...failed! Orb is nil." << endlog();
            return;
        }
        try {
            log(Info) <<"Entering orb->run()."<<endlog();
            orb->run();
            log(Info) <<"Breaking out of orb->run()."<<endlog();
        }
        catch (CORBA::Exception &e) {
            log(Error) << "Orb Run : CORBA exception raised!" << endlog();
            log() << CORBA_EXCEPTION_INFO(e) << endlog();
        }
    }

    /**
     * Class which runs an orb in an Orocos thread.
     */
    struct OrbRunner
        : public Activity
    {
        OrbRunner()
            : Activity(RTT::os::LowestPriority)
        {}
        void loop()
        {
            Logger::In in("OrbRunner");
            ControlTaskServer::RunOrb();
        }

        bool breakLoop()
        {
            return true;
        }

        void finalize()
        {
            Logger::In in("OrbRunner");
            log(Info) <<"Safely stopped."<<endlog();
        }
    };

    void ControlTaskServer::ThreadOrb()
    {
        Logger::In in("ThreadOrb");
        if ( CORBA::is_nil(orb) ) {
            log(Error) << "ThreadOrb...failed! Orb is nil." << endlog();
            return;
        }
        if (orbrunner != 0) {
            log(Error) <<"Orb already running in a thread."<<endlog();
        } else {
            log(Info) <<"Starting Orb in a thread."<<endlog();
            orbrunner = new OrbRunner();

            orbrunner->start();
        }
    }

    void ControlTaskServer::DestroyOrb()
    {
        Logger::In in("DestroyOrb");
        if ( CORBA::is_nil(orb) ) {
            log(Error) << "DestroyOrb...failed! Orb is nil." << endlog();
            return;
        }

        if (orbrunner) {
            orbrunner->stop();
            delete orbrunner;
            orbrunner = 0;
        }

        try {
            // Destroy the POA, waiting until the destruction terminates
            //poa->destroy (1, 1);
            CleanupServers();
            orb->destroy();
            log(Info) <<"Orb destroyed."<<endlog();
        }
        catch (CORBA::Exception &e) {
            log(Error) << "Orb Destroy : CORBA exception raised!" << endlog();
            log() << CORBA_EXCEPTION_INFO(e) << endlog();
        }

    }

    ControlTaskServer* ControlTaskServer::Create(TaskContext* tc, bool use_naming, bool require_name_service) {
        if ( CORBA::is_nil(orb) )
            return 0;

        if ( servers.count(tc) ) {
            log(Debug) << "Returning existing ControlTaskServer for "<<tc->getName()<<endlog();
            return servers.find(tc)->second;
        }

        // create new:
        log(Info) << "Creating new ControlTaskServer for "<<tc->getName()<<endlog();
        try {
            ControlTaskServer* cts = new ControlTaskServer(tc, use_naming, require_name_service);
            return cts;
        }
        catch( IllegalServer& is ) {
            cerr << is.what() << endl;
        }
        return 0;
    }

    CControlTask_ptr ControlTaskServer::CreateServer(TaskContext* tc, bool use_naming, bool require_name_service) {
        if ( CORBA::is_nil(orb) )
            return CControlTask::_nil();

        if ( servers.count(tc) ) {
            log(Debug) << "Returning existing ControlTaskServer for "<<tc->getName()<<endlog();
            return servers.find(tc)->second->server();
        }

        for (ControlTaskProxy::PMap::iterator it = ControlTaskProxy::proxies.begin(); it != ControlTaskProxy::proxies.end(); ++it)
            if ( (it->first) == tc ) {
                log(Debug) << "Returning server of Proxy for "<<tc->getName()<<endlog();
                return CControlTask::_duplicate(it->second);
            }

        // create new:
        log(Info) << "Creating new ControlTaskServer for "<<tc->getName()<<endlog();
        try {
            ControlTaskServer* cts = new ControlTaskServer(tc, use_naming, require_name_service);
            return cts->server();
        }
        catch( IllegalServer& is ) {
            cerr << is.what() << endl;
        }
        return CControlTask::_nil();
    }


    corba::CControlTask_ptr ControlTaskServer::server() const
    {
        // we're not a factory function, so we don't _duplicate.
        return mtask.in();
    }

}}
