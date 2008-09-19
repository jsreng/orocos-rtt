/***************************************************************************
  tag: FMTC  Tue Mar 11 21:49:22 CET 2008  ControlTaskI.cpp

                        ControlTaskI.cpp -  description
                           -------------------
    begin                : Tue March 11 2008
    copyright            : (C) 2008 FMTC
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


// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
// and
//       Institute for Software Integrated Systems
//       Vanderbilt University
//       Nashville, TN
//       USA
//       http://www.isis.vanderbilt.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html

// TAO_IDL - Generated from
// be/be_codegen.cpp:910

#include "ControlTaskI.h"
#include "ControlTaskServer.hpp"
#include "ControlTaskProxy.hpp"
#include "AttributesI.h"
#include "OperationsI.h"
#include "OperationInterfaceI.h"
#include "ScriptingAccessI.h"
#include "ServicesI.h"
#include "DataFlowI.h"
#include "Method.hpp"


using namespace RTT;
using namespace RTT::Corba;

// ControlObject:
Orocos_ControlObject_i::Orocos_ControlObject_i (RTT::OperationInterface* orig, PortableServer::POA_ptr the_poa )
    : mobj( orig ), mMFact(), mCFact(), mpoa( PortableServer::POA::_duplicate(the_poa) )
{
}

// Implementation skeleton destructor
Orocos_ControlObject_i::~Orocos_ControlObject_i (void)
{
    // FIXME free up cache ? This is done by refcountservantbase ?
//     for( CTObjMap::iterator it = ctobjmap.begin(); it != ctobjmap.end; ++it)
//         delete it->second;
}

 char* Orocos_ControlObject_i::getName (

    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
      ))
{
    CORBA::String_var ret = CORBA::string_dup( mobj->getName().c_str() );
    return ret._retn();
}

 char* Orocos_ControlObject_i::getDescription (

    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
      ))
{
    CORBA::String_var ret = CORBA::string_dup( mobj->getDescription().c_str() );
    return ret._retn();
}

::RTT::Corba::AttributeInterface_ptr Orocos_ControlObject_i::attributes (

  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    if ( CORBA::is_nil( mAttrs) ) {
        log(Debug) << "Creating AttributeInterface."<<endlog();
        try {
            Orocos_AttributeInterface_i* attrs = new Orocos_AttributeInterface_i( mobj->attributes(), mpoa.in() );
            mAttrs = attrs->_this();
        } catch( ... ) {
            log(Error) << "Failed to create AttributeInterface." <<endlog();
        }
    }
    return AttributeInterface::_duplicate( mAttrs.in() );
}

::RTT::Corba::MethodInterface_ptr Orocos_ControlObject_i::methods (

  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    if ( CORBA::is_nil( mMFact ) ) {
        log(Debug) << "Creating MethodInterface."<<endlog();
        Orocos_MethodInterface_i* mserv = new Orocos_MethodInterface_i( mobj->methods(), mpoa.in() );
        try {
            mMFact = mserv->_this();
        } catch( ... ) {
            log(Error) << "Failed to create MethodInterface." <<endlog();
        }
    }
    return MethodInterface::_duplicate( mMFact.in() );
}

::RTT::Corba::CommandInterface_ptr Orocos_ControlObject_i::commands (

  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    if ( CORBA::is_nil( mCFact ) ) {
        try {
            log(Debug) << "Creating CommandInterface."<<endlog();
            Orocos_CommandInterface_i* mserv = new Orocos_CommandInterface_i( mobj->commands(), mpoa.in() );
            mCFact = mserv->_this();
        } catch( ... ) {
            log(Error) << "Failed to create CommandInterface." <<endlog();
        }
    }
    return ::RTT::Corba::CommandInterface::_duplicate( mCFact.in() );
}

::RTT::Corba::ControlObject_ptr Orocos_ControlObject_i::getObject (
    const char * name
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string pname(name);
    // detect 'this':
    if ( pname == "this" )
        return this->_this();

    // Cache other objects
    OperationInterface* task = mobj->getObject( pname );
    if ( task ) {
        // do caching....
        Orocos_ControlObject_i* ret;
        if ( ctobjmap[pname] == 0 || ctobjmap[pname]->mobj != task) {
            // create or lookup new server for this object.
            // FIXME free up cache ? This is done by refcountservantbase ?
            //delete ctobjmap[pname];
            ctobjmap[pname] = new Orocos_ControlObject_i(task, mpoa.in() );
        }
        ret = ctobjmap[pname];
        return ret->_this();
    }
    // clear cache if possible.
    ctobjmap.erase( pname );
    return RTT::Corba::ControlObject::_nil();
}


::RTT::Corba::ObjectList * Orocos_ControlObject_i::getObjectList (

  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    TaskContext::ObjectList objects = mobj->getObjectList();
    ::RTT::Corba::ObjectList_var result = new ::RTT::Corba::ObjectList();
    result->length( objects.size() );
    for (unsigned int i=0; i != objects.size(); ++i )
        result[i] = CORBA::string_dup( objects[i].c_str() );

    return result._retn();
}

CORBA::Boolean Orocos_ControlObject_i::hasObject (
    const char * name
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string mname(name);
    return mobj->getObject(mname);
}



// Implementation skeleton constructor
Orocos_ControlTask_i::Orocos_ControlTask_i (TaskContext* orig, PortableServer::POA_ptr the_poa)
    : Orocos_ControlObject_i(orig, the_poa), mtask( orig ), mEEFact()
{
    // Add the corba object to the interface:
    mtask->methods()->addMethod(method("shutdown", &Orocos_ControlTask_i::shutdownCORBA, this),
                                   "Shutdown CORBA ORB. This function makes RunOrb() return.");

}

  void Orocos_ControlTask_i::shutdownCORBA() {
	  ControlTaskServer::ShutdownOrb(false);
  }


// Implementation skeleton destructor
Orocos_ControlTask_i::~Orocos_ControlTask_i (void)
{
}

::RTT::Corba::TaskState Orocos_ControlTask_i::getTaskState (
    )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return ::RTT::Corba::TaskState(mtask->getTaskState());
}

CORBA::Boolean Orocos_ControlTask_i::start (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->start();
}

CORBA::Boolean Orocos_ControlTask_i::stop (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->stop();
}

CORBA::Boolean Orocos_ControlTask_i::isActive (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->isActive();
}

CORBA::Boolean Orocos_ControlTask_i::isRunning (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->isRunning();
}

CORBA::Boolean Orocos_ControlTask_i::configure (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->configure();
}

CORBA::Boolean Orocos_ControlTask_i::cleanup (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->cleanup();
}

CORBA::Boolean Orocos_ControlTask_i::isConfigured (
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    return mtask->isConfigured();
}


CORBA::Boolean Orocos_ControlTask_i::resetError (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->resetError();
}


CORBA::Boolean Orocos_ControlTask_i::activate (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->activate();
}

CORBA::Boolean Orocos_ControlTask_i::inFatalError (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->inFatalError();
}


CORBA::Boolean Orocos_ControlTask_i::inRunTimeWarning (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->inRunTimeWarning();
}



CORBA::Boolean Orocos_ControlTask_i::inRunTimeError (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->inRunTimeError();
}


CORBA::Long Orocos_ControlTask_i::getWarningCount (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->getWarningCount();
}



CORBA::Long Orocos_ControlTask_i::getErrorCount (
    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    return mtask->getErrorCount();
}


::RTT::Corba::ScriptingAccess_ptr Orocos_ControlTask_i::scripting (

  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    if ( CORBA::is_nil( mEEFact ) ) {
        log(Debug) << "Creating ScriptingAccess."<<endlog();
        Orocos_ScriptingAccess_i* mserv = new Orocos_ScriptingAccess_i( mtask->scripting(), mpoa.in() );
        mEEFact = mserv->_this();
    }
    return ::RTT::Corba::ScriptingAccess::_duplicate( mEEFact.in() );
}

::RTT::Corba::ServiceInterface_ptr Orocos_ControlTask_i::services (

    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    if ( CORBA::is_nil( mService ) ) {
        log(Debug) << "Creating Services."<<endlog();
        RTT_Corba_ServiceInterface_i* mserv = new RTT_Corba_ServiceInterface_i();
        mService = mserv->_this();
    }
    return ::RTT::Corba::ServiceInterface::_duplicate( mService.in() );
}

::RTT::Corba::DataFlowInterface_ptr Orocos_ControlTask_i::ports (

    )
    ACE_THROW_SPEC ((
      CORBA::SystemException
    ))
{
    if ( CORBA::is_nil( mDataFlow ) ) {
        log(Debug) << "Creating DataFlowInterface."<<endlog();
        RTT_Corba_DataFlowInterface_i* mserv = new RTT_Corba_DataFlowInterface_i( mtask->ports() );
        mDataFlow = mserv->_this();
    }
    return ::RTT::Corba::DataFlowInterface::_duplicate( mDataFlow.in() );
}



::RTT::Corba::ControlTask::ControlTaskNames * Orocos_ControlTask_i::getPeerList (

  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    TaskContext::PeerList peers = mtask->getPeerList();
    ::RTT::Corba::ControlTask::ControlTaskNames_var result = new ::RTT::Corba::ControlTask::ControlTaskNames();
    result->length( peers.size() );
    for (unsigned int i=0; i != peers.size(); ++i )
        result[i] = CORBA::string_dup( peers[i].c_str() );

    return result._retn();
}


::RTT::Corba::ControlTask_ptr Orocos_ControlTask_i::getPeer (
    const char * name
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string pname(name);
    TaskContext* task = mtask->getPeer( pname );
    if ( task ) {
        // create or lookup new server for this peer.
        // do not export it to the naming service.
        return ControlTaskServer::CreateServer( task, false );
    }
    return RTT::Corba::ControlTask::_nil();
}

CORBA::Boolean Orocos_ControlTask_i::addPeer (
    ::RTT::Corba::ControlTask_ptr p,
    const char * alias
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string malias(alias);
    if (mtask->hasPeer(alias) == false )
        return mtask->addPeer( ControlTaskProxy::Create(p), alias );
    return false;
}

CORBA::Boolean Orocos_ControlTask_i::hasPeer (
    const char * name
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string mname(name);
    return mtask->hasPeer(mname);
}

CORBA::Boolean Orocos_ControlTask_i::removePeer (
    const char * name
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string mname(name);
    mtask->removePeer( mname );
    return true;
}

CORBA::Boolean Orocos_ControlTask_i::connectPeers (
    ::RTT::Corba::ControlTask_ptr p
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    TaskContext* t = ControlTaskProxy::Create( p );
    return mtask->connectPeers( t );
}

CORBA::Boolean Orocos_ControlTask_i::disconnectPeers (
    const char * name
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    std::string pname(name);
    mtask->disconnectPeers( pname );
    return true;
}

CORBA::Boolean Orocos_ControlTask_i::connectPorts (
    ::RTT::Corba::ControlTask_ptr p
  )
  ACE_THROW_SPEC ((
    CORBA::SystemException
  ))
{
    TaskContext* t = ControlTaskProxy::Create( p );
    return mtask->connectPorts( t );
}


