/***************************************************************************
  tag: FMTC  do nov 2 13:06:18 CET 2006  Services.hpp

                        Services.hpp -  description
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


#ifndef ORO_CORBA_SERVICES_HPP
#define ORO_CORBA_SERVICES_HPP

#include "corba.h"
#ifdef CORBA_IS_TAO
#include "orbsvcs/CosNamingC.h"
#else
#include "omniORB4/Naming.hh"
#endif


namespace RTT
{ namespace corba {

    /**
     * Helper class which locates common CORBA services.
     */
    class RTT_CORBA_API Services
    {
    protected:
        //! Handle to the name service.
        static CosNaming::NamingContext_var naming_context_;

    public:

        /**
         * Set this variable to false if the Orb may not be
         * shutdown. The default is \a true.
         */
        static bool MayShutdown;

        /**
         * Lookup and return the root naming context ("NameService").
         */
        static CosNaming::NamingContext_ptr getNamingContext();
    };

}
}

#endif
