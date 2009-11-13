/***************************************************************************
  tag: Peter Soetens  Tue Dec 21 22:43:07 CET 2004  CommandDispatch.hpp

                        CommandDispatch.hpp -  description
                           -------------------
    begin                : Tue December 21 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

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

#ifndef EXECUTION_COMMAND_DISPATCH
#define EXECUTION_COMMAND_DISPATCH

#include "../internal/DataSource.hpp"
#include "../base/DispatchInterface.hpp"
#include "../base/ConditionInterface.hpp"


namespace RTT
{ namespace scripting {

    /**
     * Dispatch a base::ActionInterface to a CommandProcessor.
     * The execute() method will return false from the moment
     * on that the dispatched command failed. Hence, this execute()
     * must not wrapped in an AsyncCommandDecorator, but directly
     * executed by the caller.
     * @deprecated This class is nowhere used and to low level for users.
     */
    class RTT_API CommandDispatch
        : public base::DispatchInterface
    {
        bool send;
        bool maccepted;
        internal::CommandProcessor* proc;
        base::ActionInterface* com;
        base::ConditionInterface* mcn;

        /**
         * Helper class which is sent to the internal::CommandProcessor
         * in order to execute the command.
         */
        struct Dispatcher : public base::ActionInterface {
            bool mexecuted, mvalid;
            CommandDispatch* cd;
            // trick: set mvalid to true internally but return in valid(): mexecuted && mvalid.
            Dispatcher(CommandDispatch* c) : mexecuted(false), mvalid(true), cd(c) {}
            void readArguments() {}
            bool execute() { mvalid=cd->com->execute(); mexecuted=true; return mvalid;}
            void reset() { mexecuted=false; mvalid=true; }
            virtual Dispatcher* clone() const { return new Dispatcher(cd); }
        } dispatcher;

    public:
        /**
         * Create a command to dispatch another command \a c to a internal::CommandProcessor \a p.
         */
        CommandDispatch(internal::CommandProcessor* p, base::ActionInterface* c, base::ConditionInterface* cn );

        /**
         * Be sure only to delete this command if the target processor is
         * not processing the encapsulated command.
         */
        ~CommandDispatch();

        void readArguments();

        bool ready() const;

        bool execute();

        /**
         * Dispatch a command. If it is not accepted, fail, if it is accepted,
         * return the dispatched command's result status.
         */
        bool dispatch();

        /**
         * After reset(), another attempt to dispatch
         * the command will be made upon execute().
         */
        void reset();

        /**
         * Returns true if the command was sent to the CommandProcessor.
         * You can use this flag to check whether execute() was invoked.
         */
        bool sent() const;

        /**
         * Returns true if the command was accepted when sent to the CommandProcessor.
         * A Command is accepted when the internal::CommandProcessor was running and its queue
         * was not full.
         */
        bool accepted() const;

        /**
         * Returns true if the command was executed by the CommandProcessor.
         * When executed() is true, you can check its result().
         */
        bool executed() const;

        /**
         * Returns true if the command was valid, i.e. the command itself
         * was executed and returned true.
         */
        bool valid() const;

        /**
         * Evaluate if the command is done.
         * @retval true if accepted(), valid() was true and the
         * completion condition was true as well.
         * @retval false otherwise.
         */
        bool done() const;

        base::DispatchInterface::Status status() const;

        virtual base::ConditionInterface* createCondition() const;

        base::DispatchInterface* clone() const;

        base::DispatchInterface* copy( std::map<const base::DataSourceBase*, base::DataSourceBase*>& alreadyCloned ) const;
    };
}}

#endif
