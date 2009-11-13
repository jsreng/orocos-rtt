/***************************************************************************
  tag: FMTC  do nov 2 13:06:18 CET 2006  ServicesI.h

                        ServicesI.h -  description
                           -------------------
    begin                : do november 02 2006
    copyright            : (C) 2006 FMTC
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


#ifndef ORO_CORBA_SERVICES_I_HPP
#define ORO_CORBA_SERVICES_I_HPP

#include "Services.hpp"
#include "ServicesC.h"
#include "corba.h"
#ifdef CORBA_IS_TAO
#include "ServicesS.h"
#endif

#include "../../Logger.hpp"
#include "ControlTaskServer.hpp"
#include <string>

/**
 * Implementation.
 */
class RTT_Corba_CServiceInterface_i
    : public virtual POA_RTT::corba::CServiceInterface,
      public virtual PortableServer::RefCountServantBase
{
    PortableServer::POA_var mpoa;

public:
    RTT_Corba_CServiceInterface_i(PortableServer::POA_ptr poa)
	  : mpoa(PortableServer::POA::_duplicate(poa)) {}

    PortableServer::POA_ptr _default_POA()
    { return PortableServer::POA::_duplicate(mpoa); }

    virtual char* getLogMessage()
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
    {
        std::string ret = RTT::log().getLogLine();
        return CORBA::string_dup( ret.c_str() );
    }

    virtual CORBA::Boolean requestShutdown()
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
    {
        if ( RTT::corba::Services::MayShutdown == false)
            return false;
        RTT::corba::ControlTaskServer::ShutdownOrb(false);
        return true;
    }

};

#endif
