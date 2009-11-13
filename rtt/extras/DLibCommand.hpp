/***************************************************************************
  tag: FMTC  do nov 2 13:06:04 CET 2006  DLibCommand.hpp

                        DLibCommand.hpp -  description
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


#ifndef ORO_DLIB_COMMAND_HPP
#define ORO_DLIB_COMMAND_HPP

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <string>
#include "../base/DispatchInterface.hpp"
#include "../internal/CommandFunctors.hpp"
#include "../internal/Invoker.hpp"
#include "../base/CommandBase.hpp"

namespace RTT
{ namespace extras {

    /**
     * This Namespace contains helper classes for the minimal and real-time
     * 'Distribution Library' of Orocos. The implementation of such a
     * library is currently not provided by the Real-Time Toolkit.
     */
    namespace DLib
    {
        /**
         * The DistributionLibrary (DL) Command wrapper class.  It
         * maps the Command C++ interface to the 'C' api.  The
         * implementation dispatches a Command through the
         * 'Distribution Library' to a remote CommandProcessor.  It
         * assumes the presense of a 'sendCommand' function which
         * takes the same arguments as the command.
         *
         * @param CommandT
         * The function signature of the command. For example, bool(
         * int, Frame, double)
         *
         */
        template<class CommandT, class ProtocolT>
        class DLibCommandImpl
            : public base::CommandBase<CommandT>
        {
        protected:
            int id;
            ProtocolT* protocol;
        public:
            /**
             * The defaults are reset by the constructor.
             */
            DLibCommandImpl()
                : id(0), protocol(0)
            {
            }

            /**
             * Call this operator if the DLibCommand takes no arguments.
             *
             * @return true if ready and succesfully sent.
             */
            bool invoke() {
                if (id && (this->protocol->getCommandStatus(id) & ( base::DispatchInterface::Ready | base::DispatchInterface::Done) ) )
                    return protocol->sendCommand(id);
                return false;
            }

            template<class T1>
            bool invoke( T1 a1 ) {
                if (id && (this->protocol->getCommandStatus(id) & ( base::DispatchInterface::Ready | base::DispatchInterface::Done) ) )
                    return protocol->sendCommand(id, a1);
                return false;
            }

            template<class T1, class T2>
            bool invoke( T1 a1, T2 a2 ) {
                if (id && (this->protocol->getCommandStatus(id) & ( base::DispatchInterface::Ready | base::DispatchInterface::Done) ) )
                    return protocol->sendCommand(id, a1, a2);
                return false;
            }

            template<class T1, class T2, class T3>
            bool invoke( T1 a1, T2 a2, T3 a3 ) {
                if (id && (this->protocol->getCommandStatus(id) & ( base::DispatchInterface::Ready | base::DispatchInterface::Done) ) )
                    return protocol->sendCommand(id, a1, a2, a3);
                return false;
            }

            template<class T1, class T2, class T3, class T4>
            bool invoke( T1 a1, T2 a2, T3 a3, T4 a4 ) {
                if (id && (this->protocol->getCommandStatus(id) & ( base::DispatchInterface::Ready | base::DispatchInterface::Done) ) )
                    return protocol->sendCommand(id, a1, a2, a3, a4);
                return false;
            }
        };


        /**
         * A Command which is dispatched using the 'Distribution Library' to a CommandProcessor.
         * It inherits from the DLibCommandImpl class.
         * @param CommandT The function signature of the command. For example,
         * bool( int, Frame, double)
         * @param ProtocolT A class containing static functions which implement the
         * DLib Command Protocol.
         */
        template<class CommandT, class ProtocolT>
        class DLibCommand
            : public internal::Invoker<CommandT,DLibCommandImpl<CommandT,ProtocolT> >
        {
        public:
            typedef CommandT Signature;

            /**
             * Create a DLibCommand object which executes a remote command
             *
             * @param component The name of the target component.
             * @param name The name of this command.
             */
            DLibCommand(std::string component, std::string name, ProtocolT* _protocol)
            {
            	this->protocol = _protocol;
                this->id = this->protocol->getCommandId(component,name);
                if (this->id == 0) {
                    log(Error) << "Could not find Component '"<<component <<"' or Command '"<<name<<"' in that component."<<endlog();
                }
            }

            virtual void readArguments() {}

            virtual bool ready() const {
                unsigned int st = this->protocol->getCommandStatus(this->id);
                return st == base::DispatchInterface::Ready || st == base::DispatchInterface::Done;
            }

            virtual bool dispatch() {
                return false;
            }

            virtual bool execute() {
                return false;
            }

            virtual bool done() const {
                return this->protocol->getCommandStatus(this->id) == base::DispatchInterface::Done;
            }

            virtual void reset() {
                return this->protocol->resetCommand(this->id);
            }

            virtual bool sent() const {
                return this->protocol->getCommandStatus(this->id) >= base::DispatchInterface::Sent;
            }

            virtual bool accepted() const {
                return this->protocol->getCommandStatus(this->id) >= base::DispatchInterface::Accepted;
            }

            virtual bool executed() const {
                return this->protocol->getCommandStatus(this->id) >= base::DispatchInterface::Executed;
            }

            virtual bool valid() const {
                return this->protocol->getCommandStatus(this->id) >= base::DispatchInterface::Valid;
            }

            virtual base::ConditionInterface* createCondition() const
            {
                return new internal::ConditionFunctor<bool(void)>( boost::bind<bool>( boost::mem_fn(&DLibCommand::done), this) );
            }

            /**
             * Creates a clone of this DLibCommand object.
             * Use this method to get a new command object
             * which has its own state information.
             *
             * @return
             */
            virtual base::DispatchInterface* clone() const {
                return new DLibCommand(*this);
            }

            virtual base::CommandBase<CommandT>* cloneI() const {
                return new DLibCommand(*this);
            }
        };
    }
}}
#endif
