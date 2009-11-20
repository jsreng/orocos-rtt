/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:09:49 CET 2006  ControlTaskProxy.cxx

                        ControlTaskProxy.cxx -  description
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

/**
 * * * * B I G  N O T E * * * *
 * This class uses the TypeTransport of Orocos which uses (void*) for
 * passing on data. TAO's CORBA implementation uses virtual inheritance,
 * which does not work well together with (void*). That is, you must cast
 * back to the exect same type the (void*) originated from and NOT to a
 * sub- or super-class. That would have been allowed without virtual inheritance.
 *
 * Hence, this class uses always the same base class (CExpression_ptr) to
 * communicate with the TypeTransport interface. Such that we know that when
 * we receive a (void*) it came from an (CExpression_ptr) and vice versa.
 *
 * Don't obey this and you'll get immediate hard to dissect crashes !
 * * * * B I G  N O T E * * * *
 */


#include "ControlTaskProxy.hpp"
#include "ControlTaskServer.hpp"
#include "ControlTaskC.h"
#include "CorbaMethodFactory.hpp"
#include "CORBAExpression.hpp"
#include "ScriptingAccessProxy.hpp"
#include "../../internal/TaskObject.hpp"

#include "../../base/CommandInterface.hpp"
#include "../../types/Types.hpp"
#include "../../extras/SequentialActivity.hpp"
#include "corba.h"
#ifdef CORBA_IS_TAO
#include "tao/TimeBaseC.h"
#include "tao/Messaging/Messaging.h"
#include "tao/Messaging/Messaging_RT_PolicyC.h"
#include "orbsvcs/CosNamingC.h"
#include <ace/String_Base.h>
#else
#include <omniORB4/Naming.hh>
#endif
#include <iostream>

#include "../../internal/TaskObject.hpp"
#include "RemotePorts.hpp"



using namespace std;
using namespace RTT::detail;

namespace RTT
{namespace corba
{
    IllegalServer::IllegalServer() : reason("This server does not exist or has the wrong type.") {}

    IllegalServer::IllegalServer(const std::string& r) : reason(r) {}

    IllegalServer::~IllegalServer() throw() {}

    const char* IllegalServer::what() const throw() { return reason.c_str(); }


    std::map<ControlTaskProxy*, corba::CControlTask_ptr> ControlTaskProxy::proxies;

    PortableServer::POA_var ControlTaskProxy::proxy_poa;

    ControlTaskProxy::~ControlTaskProxy()
    {
        log(Info) << "Terminating ControlTaskProxy for " <<  this->mtask_name <<endlog();
        if ( this->properties() ) {
            flattenPropertyBag( *this->properties() );
            deleteProperties( *this->properties() );
        }
        this->attributes()->clear();
        for (list<PortInterface*>::iterator it = port_proxies.begin(); it != port_proxies.end(); ++it)
            delete *it;
        proxies.erase(this);
    }

    ControlTaskProxy::ControlTaskProxy(std::string name, bool is_ior)
        : TaskContext("NotFound")
    {
        Logger::In in("ControlTaskProxy");
        this->clear();
        this->setActivity( new SequentialActivity() );
        try {
            if (is_ior) {
                // Use the first argument to create the task object reference,
                // in real applications we use the naming service, but let's do
                // the easy part first!
                CORBA::Object_var task_object =
                    orb->string_to_object ( name.c_str() );

                // Now downcast the object reference to the appropriate type
                mtask = corba::CControlTask::_narrow (task_object.in ());
            } else {
                // NameService
                CORBA::Object_var rootObj;
                CosNaming::NamingContext_var rootContext;
                try {
                    rootObj = orb->resolve_initial_references("NameService");
                    rootContext = CosNaming::NamingContext::_narrow(rootObj);
                } catch (...) {}

                if (CORBA::is_nil(rootContext)) {
                    std::string err("ControlTaskProxy could not acquire NameService.");
                    log(Error) << err <<endlog();
                    throw IllegalServer(err);
                }
                Logger::log() <<Logger::Debug << "ControlTaskProxy found CORBA NameService."<<endlog();
                CosNaming::Name serverName;
                serverName.length(2);
                serverName[0].id = CORBA::string_dup("ControlTasks");
                serverName[1].id = CORBA::string_dup( name.c_str() );

                // Get object reference
                CORBA::Object_var task_object = rootContext->resolve(serverName);
                mtask = corba::CControlTask::_narrow (task_object.in ());
            }
            if ( CORBA::is_nil( mtask ) ) {
                std::string err("Failed to acquire ControlTaskServer '"+name+"'.");
                Logger::log() << Logger::Error << err <<endlog();
                throw IllegalServer(err);
            }
            CORBA::String_var nm = mtask->getName(); // force connect to object.
            std::string newname( nm.in() );
            this->mtask_name = newname;
            Logger::log() << Logger::Info << "Successfully connected to ControlTaskServer '"+newname+"'."<<endlog();
            proxies[this] = mtask;
        }
        catch (CORBA::Exception &e) {
            log(Error)<< "CORBA exception raised when resolving Object !" << endlog();
            Logger::log() << CORBA_EXCEPTION_INFO(e) << endlog();
            throw;
        }
        catch (IllegalServer& e) {
            // rethrow
            throw e;
        }
        catch (...) {
            log(Error) <<"Unknown Exception in ControlTaskProxy construction!"<<endlog();
            throw;
        }

        this->synchronizeOnce();
    }

    ControlTaskProxy::ControlTaskProxy( ::RTT::corba::CControlTask_ptr taskc)
        : TaskContext("CORBAProxy"), mtask( corba::CControlTask::_duplicate(taskc) )
    {
        Logger::In in("ControlTaskProxy");
        this->clear();
        this->setActivity( new SequentialActivity() );
        try {
            CORBA::String_var nm = mtask->getName(); // force connect to object.
            std::string name( nm.in() );
            this->mtask_name = name;
            proxies[this] = mtask;
        }
        catch (CORBA::Exception &e) {
            log(Error) << "CORBA exception raised when creating ControlTaskProxy!" << Logger::nl;
            Logger::log() << CORBA_EXCEPTION_INFO(e) << endlog();
        }
        catch (...) {
            throw;
        }
        this->synchronizeOnce();
    }

    void ControlTaskProxy::synchronizeOnce()
    {
        // Add here the interfaces that need to be synchronised only once at creation time.
        if (CORBA::is_nil(mtask))
            return;
        log(Info) << "Creating Proxy interface for " << mtask_name <<endlog();
        log(Debug) << "Fetching CScriptingAccess."<<endlog();
        corba::CScriptingAccess_var saC = mtask->scripting();
        if ( saC ) {
            delete mscriptAcc;
            mscriptAcc = new ScriptingAccessProxy( saC.in() );
        }

        this->synchronize();
    }

    void ControlTaskProxy::synchronize()
    {
        // Add here the interfaces that need to be synchronised every time a lookup is done.
        // Detect already added parts of an interface, does not yet detect removed parts...
        if (CORBA::is_nil(mtask))
            return;

        // load command and method factories.
        // methods:
        log(Debug) << "Fetching Methods."<<endlog();
        CMethodInterface_var mfact = mtask->methods();
        if (mfact) {
            CMethodList_var objs;
            objs = mfact->getMethods();
            for ( size_t i=0; i < objs->length(); ++i) {
                if (this->methods()->hasMember( string(objs[i].in() )))
                    continue; // already added.
                this->methods()->add( objs[i].in(), new CorbaMethodFactory( objs[i].in(), mfact.in(), ProxyPOA() ) );
            }
        }

        // first do properties:
      log(Debug) << "Fetching Properties."<<endlog();
      CAttributeInterface_var attrint = mtask->attributes();
        CAttributeInterface::CPropertyNames_var props = attrint->getPropertyList();

        for (size_t i=0; i != props->length(); ++i) {
            if ( this->attributes()->hasProperty( string(props[i].name.in()) ) )
                continue; // previously added.
            CExpression_var expr = attrint->getProperty( props[i].name.in() );
            if ( CORBA::is_nil( expr ) ) {
                log(Error) <<"Property "<< string(props[i].name.in()) << " present in getPropertyList() but not accessible."<<endlog();
                continue;
            }
#if 0 // This code may trigger endless recurse if server has recursive prop bags.
      // By using Property<PropertyBag>::narrow( ... ) this is no longer needed.
            // See if it is a PropertyBag:
            CORBA::Any_var any = expr->get();
            PropertyBag bag;
            if ( AnyConversion<PropertyBag>::update( *any, bag ) ) {
                Property<PropertyBag>* pbag = new Property<PropertyBag>( string(props[i].name.in()), string(props[i].description.in()), bag);
                this->attributes()->addProperty( pbag );
                continue;
            }
#endif
            CAssignableExpression_var as_expr = CAssignableExpression::_narrow( expr.in() );
            // addProperty also adds as attribute...
            if ( CORBA::is_nil( as_expr ) ) {
                log(Error) <<"Property "<< string(props[i].name.in()) << " was not writable !"<<endlog();
            } else {
                // If the type is known, immediately build the correct property and datasource,
                // otherwise, build a property of CORBA::Any.
                CORBA::String_var tn = as_expr->getTypeName();
                TypeInfo* ti = TypeInfoRepository::Instance()->type( tn.in() );
                Logger::log() <<Logger::Info << "Looking up Property " << tn.in();
                if ( ti && ti->getProtocol(ORO_CORBA_PROTOCOL_ID)) {
                    this->attributes()->addProperty( ti->buildProperty( props[i].name.in(), props[i].description.in(),
                                                                        ti->getProtocol(ORO_CORBA_PROTOCOL_ID)->proxy( expr.in() ) ) );
                    Logger::log() <<Logger::Info <<" found!"<<endlog();
                }
                else {
                    this->attributes()->addProperty( new Property<CORBA::Any_ptr>( string(props[i].name.in()), string(props[i].description.in()), new CORBAAssignableExpression<Property<CORBA::Any_ptr>::DataSourceType>( as_expr.in() ) ) );
                    Logger::log()  <<Logger::Info<<" not found :-("<<endlog();
                }
            }
        }

      log(Debug) << "Fetching Attributes."<<endlog();
        // add attributes not yet added by properties:
        CAttributeInterface::CAttributeNames_var attrs = attrint->getAttributeList();

        for (size_t i=0; i != attrs->length(); ++i) {
            if ( this->attributes()->hasAttribute( string(attrs[i].in()) ) )
                continue; // previously added.
            CExpression_var expr = attrint->getAttribute( attrs[i].in() );
            if ( CORBA::is_nil( expr ) ) {
                log(Error) <<"Attribute "<< string(attrs[i].in()) << " present in getAttributeList() but not accessible."<<endlog();
                continue;
            }
            CAssignableExpression_var as_expr = CAssignableExpression::_narrow( expr.in()  );
            // If the type is known, immediately build the correct attribute and datasource,
            // otherwise, build a attribute of CORBA::Any.
            CORBA::String_var tn = expr->getTypeName();
            TypeInfo* ti = TypeInfoRepository::Instance()->type( tn.in() );
            log(Info) << "Looking up Attribute " << tn.in();
            if ( ti && ti->getProtocol(ORO_CORBA_PROTOCOL_ID) ) {
                Logger::log() <<": found!"<<endlog();
                if ( CORBA::is_nil( as_expr ) ) {
                    this->attributes()->setValue( ti->buildConstant( attrs[i].in(), ti->getProtocol(ORO_CORBA_PROTOCOL_ID)->proxy( expr.in() ) ) );
                }
                else {
                    this->attributes()->setValue( ti->buildAttribute( attrs[i].in(), ti->getProtocol(ORO_CORBA_PROTOCOL_ID)->proxy( expr.in() ) ) );
                }
            } else {
                Logger::log() <<": not found :-("<<endlog();
                if ( CORBA::is_nil( as_expr ) )
                    this->attributes()->setValue( new Constant<CORBA::Any_ptr>( attrs[i].in(), new CORBAExpression<CORBA::Any_ptr>( expr.in() ) ) );
                else
                    this->attributes()->setValue( new Attribute<CORBA::Any_ptr>( attrs[i].in(), new CORBAAssignableExpression<CORBA::Any_ptr>( as_expr.in() ) ) );
            }
        }

        log(Debug) << "Fetching Ports."<<endlog();
        CDataFlowInterface_var dfact = mtask->ports();
        TypeInfoRepository::shared_ptr type_repo = TypeInfoRepository::Instance();
        if (dfact) {
            CDataFlowInterface::CPortDescriptions_var objs = dfact->getPortDescriptions();
            for ( size_t i=0; i < objs->length(); ++i) {
                CPortDescription port = objs[i];
                if (this->ports()->getPort( port.name.in() ))
                    continue; // already added.

                TypeInfo const* type_info = type_repo->type(port.type_name.in());
                if (!type_info)
                {
                    log(Warning) << "remote port " << port.name
                        << " has a type that cannot be marshalled over CORBA: " << port.type_name << ". "
                        << "It is ignored by ControlTaskProxy" << endlog();
                }
                else
                {
                    PortInterface* new_port;
                    if (port.type == RTT::corba::CInput)
                        new_port = new RemoteInputPort( type_info, dfact.in(), port.name.in(), ProxyPOA() );
                    else
                        new_port = new RemoteOutputPort( type_info, dfact.in(), port.name.in(), ProxyPOA() );

                    this->ports()->addPort(new_port);
                    port_proxies.push_back(new_port); // see comment in definition of port_proxies
                }
            }
        }

        this->fetchObjects(this, mtask.in() );

        log(Debug) << "All Done."<<endlog();
    }

    // Recursively fetch remote objects and create local proxies.
    void ControlTaskProxy::fetchObjects(OperationInterface* parent, CControlObject_ptr mtask)
    {
        log(Debug) << "Fetching Objects of "<<parent->getName()<<":"<<endlog();

        corba::CObjectList_var plist = mtask->getObjectList();

        for( size_t i =0; i != plist->length(); ++i) {
            if ( string( plist[i] ) == "this")
                continue;
            CControlObject_var cobj = mtask->getObject(plist[i]);
            CORBA::String_var descr = cobj->getDescription();

            OperationInterface* tobj = this->getObject(std::string(plist[i]));
            if (tobj == 0)
                tobj = new TaskObject( std::string(plist[i]), std::string(descr.in()) );

            // add attributes:
            log(Debug) << plist[i] << ": fetching Attributes."<<endlog();
            CAttributeInterface_var objattrs = cobj->attributes();
            CAttributeInterface::CAttributeNames_var attrs =objattrs->getAttributeList();

            for (size_t j=0; j != attrs->length(); ++j) {
                if ( tobj->attributes()->hasAttribute( string(attrs[j].in()) ) )
                    continue; // previously added.
                CExpression_var expr = objattrs->getAttribute( attrs[j].in() );
                if ( CORBA::is_nil( expr ) ) {
                    log(Error) <<"Attribute "<< string(attrs[j].in()) << " present in getAttributeList() but not accessible."<<endlog();
                    continue;
                }
                CAssignableExpression_var as_expr = CAssignableExpression::_narrow( expr.in()  );
                // If the type is known, immediately build the correct attribute and datasource,
                // otherwise, build a attribute of CORBA::Any.
                CORBA::String_var tn = expr->getTypeName();
                TypeInfo* ti = TypeInfoRepository::Instance()->type( tn.in() );
                log(Debug) << "Looking up Attribute " << tn.in();
                if ( ti && ti->getProtocol(ORO_CORBA_PROTOCOL_ID) ) {
                    Logger::log() <<": found!"<<endlog();
                    if ( CORBA::is_nil( as_expr ) ) {
                        tobj->attributes()->setValue( ti->buildConstant( attrs[j].in(), ti->getProtocol(ORO_CORBA_PROTOCOL_ID)->proxy( expr.in() ) ) );
                    }
                    else {
                        tobj->attributes()->setValue( ti->buildAttribute( attrs[j].in(), ti->getProtocol(ORO_CORBA_PROTOCOL_ID)->proxy( expr.in() ) ) );
                    }
                } else {
                    Logger::log() <<": not found :-("<<endlog();
                    if ( CORBA::is_nil( as_expr ) )
                        tobj->attributes()->setValue( new Constant<CORBA::Any_ptr>( attrs[j].in(), new CORBAExpression<CORBA::Any_ptr>( expr.in() ) ) );
                    else
                        tobj->attributes()->setValue( new Attribute<CORBA::Any_ptr>( attrs[j].in(), new CORBAAssignableExpression<CORBA::Any_ptr>( as_expr.in() ) ) );
                }
            }

            // methods:
            log(Debug) << plist[i] << ": fetching Methods."<<endlog();
            CMethodInterface_var mfact = cobj->methods();
            if (mfact) {
                CMethodList_var objs;
                objs = mfact->getMethods();
                for ( size_t i=0; i < objs->length(); ++i) {
                    if (tobj->methods()->hasMember( string(objs[i].in() )))
                        continue; // already added.
                    tobj->methods()->add( objs[i].in(), new CorbaMethodFactory( objs[i].in(), mfact.in(), ProxyPOA() ) );
                }
            }
            // Add if not yet present.
            if (parent->getObject( string(plist[i].in()) ) == 0 )
                parent->addObject( tobj );

            // Recurse:
            this->fetchObjects( tobj, cobj.in() );
        }
    }

    void ControlTaskProxy::DestroyOrb()
    {
        try {
            // Destroy the POA, waiting until the destruction terminates
            //poa->destroy (1, 1);
            orb->destroy();
            std::cerr <<"Orb destroyed."<<std::endl;
        }
        catch (CORBA::Exception &e) {
            log(Error) << "Orb Init : CORBA exception raised!" << Logger::nl;
            Logger::log() << CORBA_EXCEPTION_INFO(e) << endlog();
        }
    }

    ControlTaskProxy* ControlTaskProxy::Create(std::string name, bool is_ior /*=false*/) {
        if ( CORBA::is_nil(orb) || name.empty() )
            return 0;

        // create new:
        try {
            ControlTaskProxy* ctp = new ControlTaskProxy( name, is_ior );
            return ctp;
        }
        catch( IllegalServer& is ) {
            cerr << is.what() << endl;
        }
        return 0;
    }

    TaskContext* ControlTaskProxy::Create(::RTT::corba::CControlTask_ptr t, bool force_remote) {
        Logger::In in("ControlTaskProxy::Create");
        if ( CORBA::is_nil(orb) ) {
            log(Error) << "Can not create proxy when ORB is nill !"<<endlog();
            return 0;
        }
        if ( CORBA::is_nil(t) ) {
            log(Error) << "Can not create proxy for nill peer !" <<endlog();
            return 0;
        }

        // proxy present for this object ?
        // is_equivalent is actually our best try.
        for (PMap::iterator it = proxies.begin(); it != proxies.end(); ++it)
            if ( (it->second)->_is_equivalent( t ) ) {
                log(Debug) << "Existing proxy found !" <<endlog();
                return it->first;
            }
        
        // Check if the CControlTask is actually a local TaskContext
        if (! force_remote)
        {
            for (ControlTaskServer::ServerMap::iterator it = ControlTaskServer::servers.begin(); it != ControlTaskServer::servers.end(); ++it)
                if ( it->second->server()->_is_equivalent( t ) ) {
                    log(Debug) << "Local server found !" <<endlog();
                    return it->first;
                }
        }

        log(Debug) << "No local taskcontext found..." <<endlog();
        // create new:
        try {
            ControlTaskProxy* ctp = new ControlTaskProxy( t );
            return ctp;
        }
        catch( IllegalServer& is ) {
            cerr << is.what() << endl;
        }
        return 0;
    }

    bool ControlTaskProxy::start() {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->start();
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::stop() {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->stop();
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::activate() {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->activate();
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::resetError() {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->resetError();
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::isActive() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->isActive();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    bool ControlTaskProxy::isRunning() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->isRunning();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    bool ControlTaskProxy::configure() {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->configure();
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::cleanup() {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->cleanup();
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::isConfigured() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->isConfigured();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    bool ControlTaskProxy::inFatalError() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->inFatalError();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    bool ControlTaskProxy::inRunTimeWarning() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->inRunTimeWarning();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    bool ControlTaskProxy::inRunTimeError() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->inRunTimeError();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    int ControlTaskProxy::getErrorCount() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->getErrorCount();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return -1;
    }

    int ControlTaskProxy::getWarningCount() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return mtask->getWarningCount();
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return -1;
    }

    TaskContext::TaskState ControlTaskProxy::getTaskState() const {
        try {
            if (! CORBA::is_nil(mtask) )
                return TaskContext::TaskState( mtask->getTaskState() );
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return TaskContext::Init;
    }

    void ControlTaskProxy::setName(const std::string& n)
    {
        //mtask->setName( n.c_str() );
    }

    bool ControlTaskProxy::addPeer( TaskContext* peer, std::string alias /*= ""*/ )
    {
        try {
            if (CORBA::is_nil(mtask))
                return false;

            // if peer is a proxy, add the proxy, otherwise, create new server.
            ControlTaskProxy* ctp = dynamic_cast<ControlTaskProxy*>( peer );
            if (ctp) {
                if ( mtask->addPeer( ctp->server(), alias.c_str() ) ) {
                    this->synchronize();
                    return true;
                }
                return false;
            }
            // no server yet, create it.
            ControlTaskServer* newpeer = ControlTaskServer::Create(peer);
            if ( mtask->addPeer( newpeer->server(), alias.c_str() ) ) {
                this->synchronize();
                return true;
            }
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    void ControlTaskProxy::removePeer( const std::string& name )
    {
        try {
            if (CORBA::is_nil(mtask))
                return;
            mtask->removePeer( name.c_str() );
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
    }

    void ControlTaskProxy::removePeer( TaskContext* peer )
    {
        try {
            if (CORBA::is_nil(mtask))
                return;
            mtask->removePeer( peer->getName().c_str() );
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
    }

    bool ControlTaskProxy::connectPeers( TaskContext* peer )
    {
        try {
            if (CORBA::is_nil(mtask))
                return false;
            ControlTaskServer* newpeer = ControlTaskServer::Create(peer);
            return mtask->connectPeers( newpeer->server() );
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    void ControlTaskProxy::disconnectPeers( const std::string& name )
    {
        try {
            if (! CORBA::is_nil(mtask) )
                mtask->disconnectPeers( name.c_str() );
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
    }

    TaskContext::PeerList ControlTaskProxy::getPeerList() const
    {

        TaskContext::PeerList vlist;
        try {
            if (! CORBA::is_nil(mtask) ) {
                corba::CControlTask::CControlTaskNames_var plist = mtask->getPeerList();
                for( size_t i =0; i != plist->length(); ++i)
                    vlist.push_back( std::string( plist[i] ) );
            }
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return vlist;
    }

    bool ControlTaskProxy::hasPeer( const std::string& peer_name ) const
    {
        try {
            if (! CORBA::is_nil(mtask))
                return mtask->hasPeer( peer_name.c_str() );
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return false;
    }

    TaskContext* ControlTaskProxy::getPeer(const std::string& peer_name ) const
    {
        try {
            if (CORBA::is_nil(mtask))
                return 0;
            corba::CControlTask_ptr ct = mtask->getPeer( peer_name.c_str() );
            if ( CORBA::is_nil(ct) )
                return 0;
            return ControlTaskProxy::Create( ct );
        } catch(...) {
            mtask = CControlTask::_nil();
        }
        return 0;
    }

    bool ControlTaskProxy::connectPorts( TaskContext* peer )
    {
        try {
            if (CORBA::is_nil(mtask))
                return false;
            ControlTaskServer* newpeer = ControlTaskServer::Create(peer);
            return mtask->connectPorts( newpeer->server() );
        } catch(...) {
            mtask = CControlTask::_nil();
            this->setName("NotFound");
            this->clear();
        }
        return false;
    }

    bool ControlTaskProxy::ready()
    {
        if (CORBA::is_nil(mtask)) {
            this->clear();
            return false;
        }
        try {
            mtask->getName(); // basic check
            return true;
        } catch(...) {
            // we could also try to re-establish the connection in case of naming...
            this->clear();
            mtask = CControlTask::_nil();
        }
        return false;
    }

    corba::CControlTask_ptr ControlTaskProxy::server() const {
        if ( CORBA::is_nil(mtask) )
            return CControlTask::_nil();
        return corba::CControlTask::_duplicate(mtask);
    }

    PortableServer::POA_ptr ControlTaskProxy::ProxyPOA() {
        if ( CORBA::is_nil(orb) )
            return PortableServer::POA::_nil();
        if ( CORBA::is_nil(proxy_poa) ) {
            CORBA::Object_var poa_object =
                orb->resolve_initial_references ("RootPOA");

            // new POA for the proxies:
            // Use default manager, is already activated !
            //PortableServer::POAManager_var proxy_manager = poa->the_POAManager ();
            //CORBA::PolicyList pol;
            //proxy_poa = poa->create_POA( "ProxyPOA", proxy_manager, pol );
            proxy_poa =
                PortableServer::POA::_narrow (poa_object.in ());
        }
        // note: do not use _retn(): user must duplicate in constructor.
        return proxy_poa.in();
    }
}}

