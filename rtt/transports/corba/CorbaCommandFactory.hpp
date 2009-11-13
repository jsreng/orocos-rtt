/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:58 CEST 2006  CorbaCommandFactory.hpp

                        CorbaCommandFactory.hpp -  description
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


#ifndef ORO_CORBACOMMANDFACTORY_HPP
#define ORO_CORBACOMMANDFACTORY_HPP

#include "../../internal/OperationFactory.hpp"
#include "CommandProxy.hpp"
#include "OperationInterfaceC.h"
#include "CommandProxy.hpp"

namespace RTT
{namespace corba
{

    /**
     * This class mirrors a Corba CCommand Factory as a plain C++
     * factory.
     */
    class RTT_CORBA_API CorbaCommandFactory
        : public RTT::internal::OperationFactoryPart<base::DispatchInterface*>
    {
        std::string com;
        corba::CCommandInterface_var mfact;
        PortableServer::POA_var mpoa;
    public:
        typedef std::vector< internal::ArgumentDescription > CDescriptions;
        typedef std::vector<std::string> Commands;
        typedef std::vector<base::DataSourceBase::shared_ptr> CArguments;

        CorbaCommandFactory(const std::string& command, corba::CCommandInterface_ptr fact, PortableServer::POA_ptr the_poa)
            : RTT::internal::OperationFactoryPart<base::DispatchInterface*>("Corba CCommand"),
              com(command), mfact( CCommandInterface::_duplicate(fact) ),
              mpoa(PortableServer::POA::_duplicate(the_poa) )
        {}

        virtual ~CorbaCommandFactory() {}

        virtual int arity()  const {
            return this->getArgumentList().size();
        }

        virtual std::string resultType() const
        {
            return "bool";
        }

        virtual std::string description() const
        {
            try {
                CORBA::String_var result = mfact->getDescription( com.c_str() );
                return std::string( result.in() );
            } catch ( corba::CNoSuchNameException& nsn ) {
                throw internal:: name_not_found_exception( nsn.name.in() );
            }
            return std::string();
        }

        /**
         * @brief Return the list of arguments of a certain command.
         */
        virtual CDescriptions getArgumentList() const
        {
            CDescriptions ret;
            try {
                corba::CDescriptions_var result = mfact->getArguments( com.c_str() );
                ret.reserve( result->length() );
                for (size_t i=0; i!= result->length(); ++i)
                    ret.push_back( internal::ArgumentDescription(std::string( result[i].name.in() ),
                                                       std::string( result[i].description.in() ),
                                                       std::string( result[i].type.in() ) ));
            } catch ( corba::CNoSuchNameException& nsn ) {
                throw internal:: name_not_found_exception( nsn.name.in() );
            }
            return ret;
        }

        virtual base::DispatchInterface* produce(const CArguments& args) const
        {
            corba::CArguments_var nargs = new corba::CArguments();
            nargs->length( args.size() );
            for (size_t i=0; i < args.size(); ++i )
                nargs[i] = (corba::CExpression_ptr)args[i]->server(ORO_CORBA_PROTOCOL_ID, mpoa.in() );
            try {
                corba::CCommand_var result = mfact->createCommand( com.c_str(), nargs.in() );
                // return a base::DispatchInterface object:
                return CommandProxy::Create( result.in() );
            } catch ( corba::CNoSuchNameException& nsn ) {
                throw internal:: name_not_found_exception( nsn.name.in() );
            } catch ( corba::CWrongNumbArgException& wa ) {
                throw internal:: wrong_number_of_args_exception( wa.wanted, wa.received );
            } catch ( corba::CWrongTypeArgException& wta ) {
                throw internal:: wrong_types_of_args_exception( wta.whicharg, wta.expected.in(), wta.received.in() );
            }
            return 0; // not reached.
        }
    };
}}

#endif
