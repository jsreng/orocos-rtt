/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:09:48 CET 2006  ControlTaskServer.hpp

                        ControlTaskServer.hpp -  description
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

#ifndef ORO_CORBA_CONTROLTASK_SERVER_HPP
#define ORO_CORBA_CONTROLTASK_SERVER_HPP

#include <map>
#ifndef _REENTRANT
#define _REENTRANT
#endif
#include "corba.h"
#ifdef CORBA_IS_TAO
#include "ControlTaskS.h"
#else
#include "ControlTaskC.h"
#endif
#include "../../TaskContext.hpp"
#include "ApplicationServer.hpp"
#include "../../base/ActivityInterface.hpp"

class Orocos_CControlTask_i;
namespace RTT
{namespace corba
{
    class OrbRunner;
    class ControlTaskProxy;

    /**
     * This class manages the creation of TaskContext Corba Servers
     * and a Corba Object Request Broker (Orb) which executes the servers.
     * The Orb may be run from the main thread or in its own thread.
     * @ingroup CompIDL
     */
    class RTT_CORBA_API ControlTaskServer
        : public ApplicationServer
    {
    protected:
        friend class OrbRunner;
        friend class ControlTaskProxy;

        typedef std::map<TaskContext*, ControlTaskServer*> ServerMap;
        static ServerMap servers;
        static base::ActivityInterface* orbrunner;

        PortableServer::POA_var mpoa;

        /**
         * Private constructor which creates a new servant.
         */
        ControlTaskServer(TaskContext* taskcontext, bool use_naming, bool require_name_service);

        PortableServer::ServantBase_var mtask_i;
        corba::CControlTask_var mtask;
        TaskContext* mtaskcontext;
        bool muse_naming;

        /**
         * Internal shutdown function, used
         * by both thread and ShutdownOrb.
         */
        static void DoShutdownOrb(bool wait_for_completion = true);
    public:

        /**
        * When a ControlTaskServer is destroyed, the object reference
        * is removed from the Naming Service and the servant is deleted.
        */
        ~ControlTaskServer();

        /**
         * Invoke this method once to shutdown the Orb which is
         * running the task servers in RunOrb(). When this function
         * returns, no CORBA invocations are in progress, unless wait_for_completion is false.
         */
        static void ShutdownOrb(bool wait_for_completion = true);

        /**
         * Destroys all ControlTaskServer objects.
         */
        static void CleanupServers();

        /**
         * Invoke this method to run the orb and accept client requests.
         * Use ShutdownOrb() to break out of this method.
         */
        static void RunOrb();

        /**
         * Invoke this method to run the orb in a separate thread and accept client requests
         * from that thread.
         * Use ShutdownOrb() to break out of this method.
         */
        static void ThreadOrb();

        /**
         * Invoke this method once to cleanup the orb.
         */
        static void DestroyOrb();

        /**
         * Factory method: create a CORBA server for an existing TaskContext.
         * @param tc The TaskContext to serve.
         * @param use_naming Set to \a false in order not to use the Corba Naming Service.
         * @param require_naming Set to \a true to require that the Corba Naming Service be found.
         * @retval 0 if the ORB is not initialised, or if require_name_service==true and the
         * name service was not found
         * @return A new or previously created CORBA server for \a tc.
         */
        static ControlTaskServer* Create(TaskContext* tc, bool use_naming = true, bool require_name_service = false);

        /**
         * Factory method: create a CORBA server for an existing TaskContext.
         * Same as above, but immediately return the Corba object. Also checks if
         * \a tc is ControlTaskProxy and returns the server of the proxy if so.
         * @param tc The TaskContext to serve.
         * @param use_naming Set to \a false in order not to use the Corba Naming Service.
         * @param require_naming Set to \a true to require that the Corba Naming Service be found.
         * @retval 0 if the ORB is not initialised, or if require_name_service==true and the
         * name service was not found
         * @return A new or previously created CORBA server for \a tc. Since this
         * is a factory function, you need to store the object in a _var and don't
         * need to _duplicate it.
         */
        static CControlTask_ptr CreateServer(TaskContext* tc, bool use_naming = true, bool require_name_service = false);

        /**
         * Get the Corba Object of this CControlTask.
         * If you want to store this reference, you must \b _duplicate it.
         * This object universally identifies the remote ControlTaskServer
         * and can be used to tell other (remote) objects where to find it.
         */
        CControlTask_ptr server() const;
    };
}}
#endif
