/***************************************************************************
  tag: Peter Soetens  Mon Jun 26 13:25:58 CEST 2006  CorbaConversion.hpp

                        CorbaConversion.hpp -  description
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


#ifndef ORO_CORBA_CONVERSION_HPP
#define ORO_CORBA_CONVERSION_HPP

#include <string>
#include <vector>

#include "corba.h"
#ifdef CORBA_IS_TAO
#include <tao/Version.h>
#if TAO_MAJOR_VERSION == 1 && TAO_MINOR_VERSION <= 4
#include <tao/Any.h>
#else // TAO 1.5 and higher
#include <tao/AnyTypeCode/Any.h>
#endif
#include <tao/CORBA_String.h>
#else
#include "corba.h"
#include <omniORB4/stringtypes.h>
#endif

#include "OrocosTypesC.h"
#include "../../Logger.hpp"
#include "../../internal/DataSourceTypeInfo.hpp"
#include "AttributesC.h"
#include "CorbaLib.hpp"
#include "CorbaConnPolicy.hpp"

namespace RTT
{ namespace corba {

    /**
     * This class converts a given application-specific type
     * to a CORBA::Any object and vice versa.
     * This class works with 'template specialisation'.
     * To create your own any conversion, specialise
     * this class for your application's C++ or IDL type.
     * Also, your specialisation must be in the RTT
     * namespace in order to be found by the CORBA-enabled classes
     * such as Property or the TaskContext.
     * @param Type is the Standard C++ or IDL type which
     * is used throughout your application.
     */
    template<class Type>
    struct AnyConversion
    {
        typedef CORBA::Any CorbaType;
        typedef Type StdType;

        /**
         * Updates tp with the contents of any.
         * @param any Contains possibly data of a type convertible to tp
         * @param tp A C++ or CORBA type in which the contents of the
         * any are written.
         * @return true if the any was convertible to tp.
         */
        static bool update(const CORBA::Any& any, StdType tp) {
            Logger::log() << Logger::Debug << "Failing conversion of type "<<internal::DataSourceTypeInfo<StdType>::getType()<<"." <<Logger::endl;
            return false;
        }

        /**
         * Creates an CORBA::Any object out of a C++/IDL type.
         * @param tp The value to convert to an Any.
         * @return An Any object which contains tp or an
         * empty Any object if the conversion was not possible.
         */
        static CORBA::Any_ptr createAny( StdType tp ) {
            Logger::log() << Logger::Error << "Failing corba::Any creation of type "<<internal::DataSourceTypeInfo<StdType>::getType()<<"." <<Logger::endl;
            return new CORBA::Any();
        }
    };

    /**
     * Used for the conversion of types that are binary
     * compatible between CORBA type and std C++ type.
     */
    template<class Type, class _CorbaType = Type>
    struct AnyConversionHelper
    {
        typedef _CorbaType CorbaType;
        typedef Type StdType;
        static CorbaType toAny( Type t ) {
            //Logger::log() << Logger::Debug << "Converting type "<<internal::DataSourceTypeInfo<Type>::getType()<<" to same CORBA type." <<Logger::endl;
            return t;
        }
        static Type& fromAny( Type& t ) {
            return t;
        }
        static const Type& get(const Type& t) {
            return t;
        }

        static bool update(const CORBA::Any& any, StdType& _value) {
	    CorbaType temp;
            if ( any >>= temp ) {
                _value = temp;
                return true;
            }
            return false;
        }

        static CORBA::Any_ptr createAny( const Type& t ) {
            CORBA::Any_ptr ret = new CORBA::Any();
            //Logger::log() << Logger::Debug << "Creating corba::Any from "<<internal::DataSourceTypeInfo<Type>::getType()<<"." <<Logger::endl;
            *ret <<= toAny( static_cast<CorbaType>(t) );
            return ret;
        }

    };

    template<>
    struct RTT_CORBA_API AnyConversion<double> : public AnyConversionHelper<double> {};

    template<>
    struct RTT_CORBA_API AnyConversion<float> : public AnyConversionHelper<float> {};

    template<>
    struct RTT_CORBA_API AnyConversion<int> : public AnyConversionHelper<int, CORBA::Long> {};

    //template<>
    //struct RTT_CORBA_API AnyConversion<long> : public AnyConversionHelper<long> {};

    template<>
    struct RTT_CORBA_API AnyConversion<unsigned int> : public AnyConversionHelper<unsigned int, CORBA::ULong> {};

    template<>
    struct RTT_CORBA_API AnyConversion<CORBA::Any_ptr>
    {
        typedef CORBA::Any_ptr CorbaType;
        typedef CORBA::Any_ptr StdType;

        static bool update(const CORBA::Any& any, CORBA::Any_ptr _value) {
            //Logger::log() << Logger::Debug << "Updating type CORBA::Any_ptr with CORBA::Any." <<Logger::endl;
            *_value = any;
            return true;
        }

        static CORBA::Any_ptr createAny( CORBA::Any_ptr t ) {
            //Logger::log() << Logger::Debug << "Duplicating CORBA::Any_ptr." <<Logger::endl;
            return new CORBA::Any(*t);
        }
    };

    template<>
    struct RTT_CORBA_API AnyConversion<CORBA::Any_var>
    {
        typedef CORBA::Any_ptr CorbaType;
        typedef CORBA::Any_var StdType;

        static bool update(const CORBA::Any& any, CORBA::Any_var _value) {
            //Logger::log() << Logger::Debug << "Updating type CORBA::Any_var with CORBA::Any." <<Logger::endl;
            *_value.out() = any;
            return true;
        }

        static CORBA::Any_ptr createAny( CORBA::Any_var t ) {
            //Logger::log() << Logger::Debug << "Duplicating CORBA::Any_var." <<Logger::endl;
            return new CORBA::Any( t.in() );
        }
    };

    template<>
    struct RTT_CORBA_API AnyConversion<PropertyBag>
    {
        typedef corba::CAttributeInterface_ptr CorbaType;
        typedef PropertyBag StdType;

        static bool update(const CORBA::Any& any, StdType& _value);
        static CORBA::Any_ptr createAny( StdType t );
    };

    template<>
    struct RTT_CORBA_API AnyConversion<bool>
    {
        typedef CORBA::Boolean CorbaType;
        typedef bool StdType;
        static CORBA::Any::from_boolean toAny( bool t ) {
            //Logger::log() << Logger::Debug << "Converting type 'bool' to from_boolean." <<Logger::endl;
            return CORBA::Any::from_boolean(t);
        }
        static CORBA::Any::to_boolean fromAny( CORBA::Boolean& t ) {
            return CORBA::Any::to_boolean(t);
        }
        static StdType get(const CORBA::Boolean t) {
            return t;
        }

        static bool update(const CORBA::Any& any, StdType& _value) {
            CorbaType result;
            if ( any >>= AnyConversion<bool>::fromAny( result ) ) {
                _value = AnyConversion<bool>::get(result);
                return true;
            }
            return false;
        }

        static CORBA::Any_ptr createAny( bool t ) {
            CORBA::Any_ptr ret = new CORBA::Any();
            *ret <<= toAny( t );
            return ret;
        }
    };

    template<>
    struct RTT_CORBA_API AnyConversion<char>
    {
        typedef CORBA::Char CorbaType;
        typedef char StdType;
        static CORBA::Any::from_char toAny( StdType t ) {
            return CORBA::Any::from_char(t);
        }
        static CORBA::Any::to_char fromAny( CorbaType& t ) {
            return CORBA::Any::to_char(t);
        }
        static StdType get(const CorbaType t) {
            return t;
        }

        static bool update(const CORBA::Any& any, StdType& _value) {
            CorbaType result;
            if ( any >>= AnyConversion<StdType>::fromAny( result ) ) {
                _value = AnyConversion<StdType>::get(result);
                return true;
            }
            return false;
        }

        static CORBA::Any_ptr createAny( char t ) {
            CORBA::Any_ptr ret = new CORBA::Any();
            *ret <<= toAny( t );
            return ret;
        }
    };

    template<>
    struct RTT_CORBA_API AnyConversion<std::string>
    {
        typedef const char* CorbaType;
        typedef std::string StdType;
        static CorbaType toAny(const std::string& orig) {
            //Logger::log() << Logger::Debug << "Converting type 'string' to const char*." <<Logger::endl;
            return orig.c_str();
        }

        static StdType get(const CorbaType t) {
            return StdType( t );
        }

        static bool update(const CORBA::Any& any, StdType& _value) {
            CorbaType result;
            //Logger::log() << Logger::Debug << "Updating std::string with Any." <<Logger::endl;
            if ( any >>= result ) {
                _value = result;
                return true;
            }
            return false;
        }

        static CORBA::Any_ptr createAny( const std::string& t ) {
            CORBA::Any_ptr ret = new CORBA::Any();
            *ret <<= toAny( t );
            return ret;
        }

    };

    /**
     * This specialisation uses the toRTT() and toCORBA functions
     * for converting between RTT::ConnPolicy and RTT::corba::CConPolicy.
     */
    template<>
    struct RTT_CORBA_API AnyConversion<ConnPolicy>
    {
        typedef RTT::corba::CConnPolicy CorbaType;
        typedef RTT::ConnPolicy StdType;
        static CorbaType toAny(const StdType& orig) {
            //Logger::log() << Logger::Debug << "Converting type 'string' to const char*." <<Logger::endl;
            return toCORBA(orig);
        }

        static StdType get(const CorbaType t) {
            return toRTT( t );
        }

        static bool update(const CORBA::Any& any, StdType& _value) {
            CorbaType* result;
            //Logger::log() << Logger::Debug << "Updating std::string with Any." <<Logger::endl;
            if ( any >>= result ) {
                _value = toRTT(*result);
                return true;
            }
            return false;
        }

        static CORBA::Any_ptr createAny( const StdType& t ) {
            CORBA::Any_ptr ret = new CORBA::Any();
            *ret <<= toAny( t );
            return ret;
        }

    };

    template<>
    struct RTT_CORBA_API AnyConversion< std::vector<double> >
    {
        typedef corba::CDoubleSequence CorbaType;
        typedef std::vector<double> StdType;
        static CorbaType* toAny(const std::vector<double>& orig) {
            //Logger::log() << Logger::Debug << "Converting type 'std::vector<double>' to sequence<CORBA::Double>." <<Logger::endl;
            CorbaType* ret = new CorbaType();
            ret->length( (CORBA::ULong)(orig.size()) );
            for( size_t i = 0; i != orig.size(); ++i)
                (*ret)[(CORBA::ULong)(i)] = orig[i];
            return ret;
        }

        static StdType get(const CorbaType* t) {
            StdType ret;
            ret.resize( t->length() );
            for( size_t i = 0; i != t->length(); ++i)
                ret[i] = (*t)[(CORBA::ULong)(i)];
            return ret;
        }

        static bool update(const CORBA::Any& any, StdType& _value) {
            CorbaType* result;
            if ( any >>= result ) {
                _value.resize( result->length() );
                for( size_t i = 0; i != result->length(); ++i)
                    _value[i] = (*result)[(CORBA::ULong)(i)];
                return true;
            }
            return false;
        }

        static CORBA::Any_ptr createAny( const std::vector<double>& t ) {
            CORBA::Any_ptr ret = new CORBA::Any();
            *ret <<= toAny( t );
            return ret;
        }

    };


}}

#endif
