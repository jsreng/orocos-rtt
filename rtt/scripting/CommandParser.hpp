/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:26 CET 2004  CommandParser.hpp

                        CommandParser.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
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

#ifndef COMMANDPARSER_HPP
#define COMMANDPARSER_HPP

#include "parser-types.hpp"
#include "../PropertyBag.hpp"
#include "CommonParser.hpp"
#include "ExpressionParser.hpp"
#include "PeerParser.hpp"

#ifdef ORO_PRAGMA_INTERFACE
#pragma interface
#endif

namespace RTT
{
    namespace scripting {


  /**
   * This class parses commands.  Actually, it only parses call
   * commands, and the keyword 'nothing'.  Value Set Commands are
   * parsed by the ValueChangeParser.  It also takes care of checking
   * whether a command needs to be dispatched, and otherwise wraps the generated
   * command in an AsynchCommandDecorator.
   */
  class CommandParser
  {
    // the object that the user wants to call a method on, in the
    // current call command..
    std::string mcurobject;

    // the method the user wants to call in the current call command..
    std::string mcurmethod;

      // dispatch the command per definition or let it be context dependent.
      bool mas_action;

    base::ActionInterface* retcommand;
    base::ConditionInterface* implicittermcondition;
    base::ConditionInterface* dispatchCond;
    TaskContext* peer;

    void seenmethodname( iter_t begin, iter_t end )
      {
        std::string methodname( begin, end );
        mcurmethod = methodname;
      };

    void seenstartofcall();
    void seennopcommand();

    void seencallcommand();

    rule_t objectmethod, command, callcommand, nopcommand, arguments;

    TaskContext* context;
    CommonParser commonparser;
    ArgumentsParser* argsparser;
    ExpressionParser expressionparser;
      PeerParser peerparser;
  public:
      /**
       * Create a CommandParser operating in a taskcontext.
       * @param as_action return an action, meaning wrap the
       * created base::DispatchInterface in an internal::DispatchAction such that
       * execute() of the returned command does a base::DispatchInterface::dispatch().
       */
    CommandParser( TaskContext* context, bool as_action );
    ~CommandParser();

      bool foundObject() {
          return !mcurobject.empty();
      }
      bool foundMethod() {
          return !mcurmethod.empty();
      }

    rule_t& parser()
      {
        return command;
      };

      /**
       * Each condition, which relates to this command,
       * might be subject to wrapping.
       */
      base::ConditionInterface* wrapCondition( base::ConditionInterface* );

      /**
       * Return a condition evaluating to true if the command
       * is dispatched and accepted. Return zero if not applicable.
       * This method is needed, because the CommandParser can
       * only save state for one command. If multiple commands
       * are involved, this condition must be kept by the
       * parent of this parser and used if necessary.
       */
      base::ConditionInterface* dispatchCondition();

    /**
     * Get the parsed command.  Call the reset method if you use the
     * generated command, otherwise it will be deleted in the
     * CommandParser's destructor..
     */
    base::ActionInterface* getCommand()
      {
        return retcommand;
      }

    /**
     * The implicit termination condition associated with the command,
     * if any.  Ownership semantics are the same as for the
     * getCommand() function.
     */
    base::ConditionInterface* getImplTermCondition()
      {
        return implicittermcondition;
      }

    /**
     * resets the CommandParser, use this after you've succesfully
     * used the created command and implicit termination condition.
     */
      void reset();
  };
}}

#endif
