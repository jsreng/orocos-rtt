/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:59 CEST 2006  ExpressionServer.cxx

                        ExpressionServer.cxx -  description
                           -------------------
    begin                : Mon June 26 2006
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


#include "ExpressionServer.hpp"
#include "ApplicationServer.hpp"
#include "../../Logger.hpp"
#include "../../rtt-detail-fwd.hpp"
#include "OperationsI.h"

namespace RTT
{namespace corba
{
    using namespace detail;


    ExpressionServer::EServantMap ExpressionServer::EServants;
    ExpressionServer::EServerMap ExpressionServer::EServers;
    ExpressionServer::AServerMap ExpressionServer::AServers;
    ExpressionServer::MServerMap ExpressionServer::MServers;

    corba::CExpression_ptr ExpressionServer::CreateExpression( DataSourceBase::shared_ptr expr, PortableServer::POA_ptr p ) {
        // try to retrieve:
        corba::CExpression_ptr result = EServers[ expr ];
        if ( !CORBA::is_nil(result) )
            return corba::CExpression::_duplicate(result);
        // create new:
        if ( CORBA::is_nil(p) )
            p = ApplicationServer::rootPOA.in();
        Logger::log() <<Logger::Debug<< "Created 'Any' CExpression server for type "<< expr->getType()<<Logger::endl;
        Orocos_AnyExpression_i* newexpr = new Orocos_AnyExpression_i( expr, p );
        EServants[expr] = newexpr;
        EServers[expr] = newexpr->activate_this();
        return corba::CExpression::_duplicate( EServers[expr] );
    }


    corba::CExpression_ptr ExpressionServer::CreateAssignableExpression( DataSourceBase::shared_ptr expr, PortableServer::POA_ptr p ){
        // try to retrieve:
        corba::CAssignableExpression_ptr result = AServers[ expr ];
        if ( !CORBA::is_nil(result) )
            return corba::CExpression::_duplicate(result);
        // create new:
        if ( CORBA::is_nil(p) )
            p = ApplicationServer::rootPOA.in();
        Logger::log() <<Logger::Debug<< "Created 'Any' Assignable CExpression server for type "<< expr->getType()<<Logger::endl;
        Orocos_AnyAssignableExpression_i* newexpr = new Orocos_AnyAssignableExpression_i( expr, p );
        AServers[expr] = newexpr->activate_this();
        EServants[expr] = newexpr;
        EServers[expr] = corba::CExpression::_narrow(AServers[expr]);
        corba::CExpression_var ret = corba::CExpression::_duplicate( EServers[expr] );
        return ret._retn();
    }

    corba::CMethod_ptr ExpressionServer::CreateMethod( DataSourceBase::shared_ptr expr, MethodC* orig, PortableServer::POA_ptr p ) {
        // try to retrieve:
        corba::CMethod_ptr result = MServers[ expr ];
        if ( !CORBA::is_nil(result) )
            return corba::CMethod::_duplicate(result);
        // create new:
        if ( CORBA::is_nil(p) )
            p = ApplicationServer::rootPOA.in();
        Logger::log() <<Logger::Debug<< "Created 'Any' CMethod server for type "<< expr->getType()<<Logger::endl;
        Orocos_AnyMethod_i* newexpr = new Orocos_AnyMethod_i( *orig, expr, p );
        MServers[expr] = newexpr->activate_this();
        EServants[expr] = newexpr;
        EServers[expr] = corba::CExpression::_narrow(MServers[expr]);
        return corba::CMethod::_duplicate( MServers[expr] );
    }

    void ExpressionServer::copy( std::map<const DataSourceBase*, DataSourceBase*>& alreadyCloned )
    {
        /**
         * Inform servants. All servants are present in the EServantMap.
         */
        // check map for clones and replace with 'new'.
        EServantMap sclone_map;
        for ( EServantMap::iterator it = EServants.begin(); it != EServants.end(); ) {
            if ( alreadyCloned.count( it->first.get() ) ) {
                const DataSourceBase* old_ds = it->first.get();
                DataSourceBase* new_ds = alreadyCloned[ old_ds ];
                ++it;
                if ( old_ds == new_ds )
                    continue; // intercept non-clone cases.
                // store server in temp map.
                sclone_map[ new_ds ] = EServants[ old_ds ];
                // update servant pointer to new ds:
                sclone_map[ new_ds ]->copy( new_ds );
                // this erases 'old' iterator position.
                EServants.erase( old_ds );
            } else
                ++it;
        }
        // now populate the map with the new clones:
        EServants.insert( sclone_map.begin(), sclone_map.end() );

        /**
         * Update maps
         */
        // check map for clones and replace with 'new'.
        EServerMap eclone_map;
        for ( EServerMap::iterator it = EServers.begin(); it != EServers.end(); ) {
            if ( alreadyCloned.count( it->first.get() ) ) {
                const DataSourceBase* old_ds = it->first.get();
                DataSourceBase* new_ds = alreadyCloned[ old_ds ];
                ++it;
                if ( old_ds == new_ds )
                    continue; // intercept non-clone cases.
                // store server in temp map.
                eclone_map[ new_ds ] = EServers[ old_ds ];
                // this erases 'old' iterator position.
                EServers.erase( old_ds );
            } else
                ++it;
        }
        // now populate the map with the new clones:
        EServers.insert( eclone_map.begin(), eclone_map.end() );

        // check map for clones and replace with 'new'.
        AServerMap aclone_map;
        for ( AServerMap::iterator it = AServers.begin(); it != AServers.end(); ) {
            if ( alreadyCloned.count( it->first.get() ) ) {
                DataSourceBase* old_ds = it->first.get();
                DataSourceBase* new_ds = alreadyCloned[ old_ds ];
                ++it;
                if ( old_ds == new_ds )
                    continue; // intercept non-clone cases.
                // store server in temp map.
                aclone_map[ new_ds ] = AServers[ old_ds ];
                // this erases 'old' iterator position.
                AServers.erase( old_ds );
            } else
                ++it;
        }
        // now populate the map with the new clones:
        AServers.insert( aclone_map.begin(), aclone_map.end() );

        // check map for clones and replace with 'new'.
        MServerMap mclone_map;
        for ( MServerMap::iterator it = MServers.begin(); it != MServers.end(); ) {
            if ( alreadyCloned.count( it->first.get() ) ) {
                DataSourceBase* old_ds = it->first.get();
                DataSourceBase* new_ds = alreadyCloned[ old_ds ];
                ++it;
                if ( old_ds == new_ds )
                    continue; // intercept non-clone cases.
                // store server in temp map.
                mclone_map[ new_ds ] = MServers[ old_ds ];
                // this erases 'old' iterator position.
                MServers.erase( old_ds );
            } else
                ++it;
        }
        // now populate the map with the new clones:
        MServers.insert( mclone_map.begin(), mclone_map.end() );

    }

}}
