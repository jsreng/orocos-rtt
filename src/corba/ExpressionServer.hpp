/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:59 CEST 2006  ExpressionServer.hpp

                        ExpressionServer.hpp -  description
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


#ifndef ORO_CORBA_EXPRESSIONSERVER_HPP
#define ORO_CORBA_EXPRESSIONSERVER_HPP

#include "../DataSourceBase.hpp"
#include "OperationsC.h"
#ifdef CORBA_IS_TAO
#include <tao/PortableServer/PortableServer.h>
#else
#include <omniORB4/poa.h>
#endif


// forward decl.
class Orocos_AnyExpression_i;

namespace RTT
{namespace Corba
{

    /**
     * This class manages the access of local Expression Corba Servants and Servers.
     * Especially, it also takes care of the copy/clone semantics and informs the
     * servants when a DataSource copy() operation changed the location of a DataSource it was using.
     */
    class ExpressionServer
    {
    protected:
        typedef std::map<DataSourceBase::const_ptr, Orocos_AnyExpression_i* > EServantMap;
        typedef std::map<DataSourceBase::const_ptr, Corba::Expression_ptr > EServerMap;
        typedef std::map<DataSourceBase::shared_ptr, Corba::AssignableExpression_ptr> AServerMap;
        typedef std::map<DataSourceBase::shared_ptr, Corba::Method_ptr> MServerMap;

        /**
         * All created servants end up in this map.
         */
        static EServantMap EServants;

        /**
         * All created Expressions (or subclasses) end up in this map.
         */
        static EServerMap EServers;
        /**
         * The AssignableExpressions end up in this map as well.
         */
        static AServerMap AServers;
        /**
         * The Methods end up in this map as well.
         */
        static MServerMap MServers;

        /**
         * Private constructor. Not used.
         */
        ExpressionServer();

    public:

        /**
         * Factory method: create a CORBA server to an expression
         * @param expr The expression to serve
         * @return A new or previously created CORBA server for \a expr.
         */
        static Corba::Expression_ptr CreateExpression( DataSourceBase::shared_ptr expr, PortableServer::POA_ptr p);


        /**
         * Factory method: create a CORBA server to an assignable expression.
         * @param expr The expression to serve
         * @return A new or previously created CORBA server for \a expr.
         */
        static Corba::Expression_ptr CreateAssignableExpression( DataSourceBase::shared_ptr expr, PortableServer::POA_ptr p );

        /**
         * Factory method: create a CORBA server to a method
         * @param expr The expression to serve
         * @return A new or previously created CORBA server for \a expr.
         */
        static Corba::Method_ptr CreateMethod( DataSourceBase::shared_ptr expr, MethodC* orig, PortableServer::POA_ptr p );

        /**
         * This method informs the servants that a new DataSource needs to be used
         * because of a copy operation. It also updates the internal maps such that
         * each new lookup will resolve to the new DataSource instead of the old.
         * The old DataSource becomes 'unserved' and may thus become available for clean-up.
         */
        static void copy( std::map<const DataSourceBase*, DataSourceBase*>& alreadyCloned );
    };

}}

#endif
