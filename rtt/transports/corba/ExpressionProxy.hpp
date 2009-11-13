/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:58 CEST 2006  ExpressionProxy.hpp

                        ExpressionProxy.hpp -  description
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


#ifndef ORO_CORBA_EXPRESSIONPROXY_HPP
#define ORO_CORBA_EXPRESSIONPROXY_HPP

#include "corba.h"
#ifdef CORBA_IS_TAO
#include <tao/PortableServer/PortableServer.h>
#else
#include <omniORB4/poa.h>
#endif
#include "OperationsC.h"
#include "../../internal/DataSource.hpp"
#include "CORBAExpression.hpp"
#include "../../Logger.hpp"
#include "../../internal/DataSources.hpp"

namespace RTT
{namespace corba
{

    /**
     * This class manages the access of remote CExpression Corba Servers.
     */
    class RTT_CORBA_API ExpressionProxy
        : public base::DataSourceBase
    {
        CorbaTypeTransporter* ctp;
    public:
        typedef boost::intrusive_ptr<ExpressionProxy> shared_ptr;

        /**
         * Factory method: create a CORBA connection to an existing CExpression Object.
         * @param expr The Object to connect to.
         * @return A new or previously created CORBA proxy for \a expr.
         */
        static ExpressionProxy::shared_ptr Create(::RTT::corba::CExpression_ptr expr);

        /**
         * Factory method: create a internal::DataSource to an existing CExpression Object.
         * @param expr The Object to connect to.
         * @return A new or previously created internal::DataSource for \a expr.
         */
        static base::DataSourceBase::shared_ptr CreateDataSource(::RTT::corba::CExpression_ptr expr);


        /**
         * Create a internal::DataSource connection to an existing CExpression Object.
         * @param T the type of data the CExpression supposedly has.
         * @param expr The Object to connect to.
         * @return 0 if the CExpression is not convertible to T.
         */
        template<class T>
        static internal::DataSource<T>* NarrowDataSource(::RTT::corba::CExpression_ptr expr) {

            CORBA::Any_var any = expr->value();
            typename internal::DataSource<T>::value_t target = typename internal::DataSource<T>::value_t();
            internal::ReferenceDataSource<T> rds( target );
            rds.ref();
            types::TypeTransporter* tp = rds.getTypeInfo()->getProtocol(ORO_CORBA_PROTOCOL_ID);
            CorbaTypeTransporter* ctp = dynamic_cast<corba::CorbaTypeTransporter*>(tp);
            assert(ctp);

            if ( ctp->updateFromAny( &any.in(), &rds ) ) {
#if 0
                CORBA::String_var stype = expr->getType();
                Logger::log() <<Logger::Debug<< "Found valid conversion from server "<< stype.in()
                              <<" to local "<< internal::DataSource<T>::GetType()<<Logger::endl;
#endif
                return new CORBAExpression<T>( expr );
            }
            return 0; // not convertible.
        }

        /**
         * Create a internal::ConstantDataSource representing a read-only Any.
         * @param T the type of data the Any supposedly has.
         * @param any The any.
         * @return 0 if the Any is not convertible to T.
         */
        template<class T>
        static internal::DataSource<T>* NarrowConstant( const CORBA::Any& any) {
            // C++ language forces partial T specialisation using classes, not possible
            // with functions:
            return CreateConstantHelper<T>::Create( any );
        }

        /**
         * Create an Orocos internal::AssignableDataSource<T> proxy.
         * @param T the type of data the CExpression supposedly has.
         * @param expr The Object to connect to.
         * @return 0 if the CExpression is not convertible to T.
         */
        template<class T>
        static internal::AssignableDataSource<T>* NarrowAssignableDataSource( ::RTT::corba::CExpression_ptr expr) {

            corba::CAssignableExpression_var ret = corba::CAssignableExpression::_narrow( expr );
            if ( !CORBA::is_nil(ret) ) {
                CORBA::Any_var any = ret->value();
                typename internal::DataSource<T>::value_t target = typename internal::DataSource<T>::value_t();
                internal::ReferenceDataSource<T> rds( target );
                rds.ref();
                types::TypeTransporter* tp = rds.getTypeInfo()->getProtocol(ORO_CORBA_PROTOCOL_ID);
                CorbaTypeTransporter* ctp = dynamic_cast<corba::CorbaTypeTransporter*>(tp);
                assert(ctp);

                if ( ctp->updateFromAny( &any.in(), &rds ) ) {
#if 0
                    CORBA::String_var stype = ret->getType();
                    Logger::log() <<Logger::Debug<< "Found valid assignment conversion from server "<< stype.in()
                                  <<" to local "<< internal::DataSource<T>::GetType()<<Logger::endl;
#endif
                    return new CORBAAssignableExpression<T>( ret._retn() );
                }
            }
            return 0; // not convertible.
        }

        /**
         * Create an Orocos internal::DataSource<T> proxy.
         * @param T the type of data the CExpression supposedly has.
         * @return 0 if the CExpression is not convertible to T.
         */
        template<class T>
        internal::DataSource<T>* narrowDataSource() const {
            return NarrowDataSource<T>( mdata );
        }

        /**
         * Create an Orocos internal::DataSource<void> proxy.
         * @return A new DataSource.
         */
        internal::DataSource<void>* narrowDataSource() const {
            return new CORBAExpression<void>( mdata.in() );
        }

        /**
         * Create an Orocos internal::AssignableDataSource<T> proxy.
         * @param T the type of data the CExpression supposedly has.
         * @return 0 if the CExpression is not convertible to T.
         */
        template<class T>
        internal::AssignableDataSource<T>* narrowAssignableDataSource() const {
            return NarrowAssignableDataSource<T>( mdata.in() );
        }

        /**
         * Get the Corba Object reference of the CExpression.
         * This object universally identifies the remote CExpression Object
         * and can be used to tell other (remote) objects where to find it.
         */
        //virtual corba::CExpression_ptr createExpression() const;

        virtual int serverProtocol() const
        {
            return ORO_CORBA_PROTOCOL_ID;
        }

        virtual bool evaluate() const {
            return mdata->evaluate();
        }

        virtual base::DataSourceBase* clone() const {
            return new ExpressionProxy( mdata.in() );
        }

        virtual base::DataSourceBase* copy( std::map<const base::DataSourceBase*, base::DataSourceBase*>& alreadyCloned ) const {
            alreadyCloned[this] = const_cast<ExpressionProxy*>(this);
            return alreadyCloned[this];
        }

        virtual std::string getType() const { CORBA::String_var stype = mdata->getType() ; return std::string( stype.in() ); }

        virtual const types::TypeInfo* getTypeInfo() const { return internal::DataSourceTypeInfo<internal::UnknownType>::getTypeInfo(); }

        virtual std::string getTypeName() const { return std::string( mdata->getTypeName() ); }

        virtual void* server(int p, void* arg) {
            if (p == ORO_CORBA_PROTOCOL_ID)
                return corba::CExpression::_duplicate(mdata.in());
            return 0;
        }

        virtual void* server(int p, void* arg) const {
            if (p == ORO_CORBA_PROTOCOL_ID)
                return corba::CExpression::_duplicate(mdata.in());
            return 0;
        }

        virtual void* method(int p, internal::MethodC* , void* arg) {
            if (p == ORO_CORBA_PROTOCOL_ID)
                return corba::CMethod::_narrow( mdata.in() );
            return 0;
        }

    private:
        template<class T>
        struct CreateConstantHelper
        {
            static internal::DataSource<T>* Create(const CORBA::Any& any) {

                typename internal::DataSource<T>::value_t target = typename internal::DataSource<T>::value_t();
                internal::ReferenceDataSource<T> rds( target );
                rds.ref();
                types::TypeTransporter* tp = rds.getTypeInfo()->getProtocol(ORO_CORBA_PROTOCOL_ID);
                CorbaTypeTransporter* ctp = dynamic_cast<corba::CorbaTypeTransporter*>(tp);
                assert(ctp);

                if ( ctp->updateFromAny( &any, &rds ) ) {
                    Logger::log() <<Logger::Debug<< "Found valid conversion from CORBA::Any "
                                  <<" to local constant "<< internal::DataSource<T>::GetType()<<Logger::endl;
                    return new internal::ConstantDataSource<T>( target );
                }
                return 0; // not convertible.
            }
        };

        template<class T>
        struct CreateConstantHelper<T&>
        {
            static internal::DataSource<T&>* Create(const CORBA::Any& any) {
                return 0; // not convertible.
            }
        };

        template<class T>
        struct CreateConstantHelper<const T&>
        {
            static internal::DataSource<const T&>* Create(const CORBA::Any& any) {

                typename internal::DataSource<T>::value_t target = typename internal::DataSource<T>::value_t();
                internal::ReferenceDataSource<T> rds( target );
                rds.ref();
                if ( rds.updateBlob(ORO_CORBA_PROTOCOL_ID, &any ) ) {
                    Logger::log() <<Logger::Debug<< "Found valid conversion from CORBA::Any "
                                  <<" to local constant "<< internal::DataSource<const T&>::GetType()<<Logger::endl;
                    return new internal::ConstantDataSource<const T&>( target );
                }
                return 0; // not convertible.
            }
        };

    protected:
        typedef std::map<corba::CExpression_ptr, ExpressionProxy::shared_ptr> EMap;
        typedef std::map<corba::CExpression_ptr, base::DataSourceBase::shared_ptr> DMap;

        static EMap proxies;
        static DMap dproxies;

        /**
         * Private constructor which creates a new connection to
         * a corba object
         */
        ExpressionProxy( ::RTT::corba::CExpression_ptr t );

        corba::CExpression_var mdata;

    };

}}

#endif
