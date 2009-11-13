/***************************************************************************
  tag: Peter Soetens  Mon May 10 19:10:37 CEST 2004  ExpressionParser.cxx

                        ExpressionParser.cxx -  description
                           -------------------
    begin                : Mon May 10 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#include "parser-debug.hpp"
#include "parse_exception.hpp"

#ifdef ORO_PRAGMA_INTERFACE
#pragma implementation
#endif
#include "ExpressionParser.hpp"
//#include "DumpObject.hpp"

#include "ArgumentsParser.hpp"
#include "../types/Operators.hpp"
#include "DataSourceCondition.hpp"

#include "DataSourceTime.hpp"
#include "../TaskContext.hpp"
#include "PeerParser.hpp"
#include "../types/Types.hpp"

#include <boost/lambda/lambda.hpp>

#include <boost/bind.hpp>
#include "../rtt-config.h"
#include <iostream>

namespace RTT
{
    using boost::bind;

    using namespace detail;
    using namespace std;

    namespace {
        assertion<std::string> expect_open("Open brace expected.");
        assertion<std::string> expect_close("Closing brace expected ( or could not find out what this line means ).");
        assertion<std::string> expect_type("Unknown type. Please specify a type.");
        assertion<std::string> expect_expr("Expected a valid expression.");
        assertion<std::string> expect_ident("Expected a valid identifier.");
        assertion<std::string> expect_init("Expected an initialisation value of the value.");
        assertion<std::string> expect_comma("Expected the ',' separator after expression.");
        assertion<std::string> expect_timespec("Expected a time specification (e.g. > 10s or > varname ) after 'time' .");

        guard<std::string> my_guard;
    }



  DataCallParser::DataCallParser( ExpressionParser& p, TaskContext* c )
      : expressionparser( p ), peerparser( c )
  {
    BOOST_SPIRIT_DEBUG_RULE( datacall );
    BOOST_SPIRIT_DEBUG_RULE( arguments );

    // this parser uses a neat boost.spirit trick to avoid keeping
    // loads of stacks for all parsing data ( this parser needs to be
    // reentrant because it can be called while parsing an argument of
    // a datacall, which has itself been called while parsing an
    // argument of a datacall... ).  Boost.Spirit allows you to change
    // the parser that a rule points to at runtime, so we only create
    // the parser just before it's going to be used, when we know what
    // arguments we want..  See the ArgumentsParser doc for more
    // details..
    datacall =
        ( peerparser.locator() >>
          !(commonparser.identifier >> ".") >>   // just consume, peer locator already has object name
          expect_ident(commonparser.identifier)[bind( &DataCallParser::seenmethodname, this, _1, _2 ) ]
          [ bind( &DataCallParser::seendataname, this ) ]
          >> !arguments
          )[ bind( &DataCallParser::seendatacall, this ) ];
  };


  void DataCallParser::seendataname()
  {
      mobject =  peerparser.object();
      TaskContext* peer = peerparser.peer();
      OperationInterface* ops  = peerparser.taskObject();
      peerparser.reset();

      // Check if it is a constructor
      if ( mobject == "this" && TypeInfoRepository::Instance()->type( mmethod ) ) {
          // it is...
      } else {
          // it ain't...
          //cout << "DCP saw method "<< mmethod <<" of object "<<mobject<<" of peer "<<peer->getName()<<endl;
          // this is slightly different from CommandParser
          if ( ops == 0 ) {
              //DumpObject( peer );
              throw_( iter_t(), std::string("Task '")+peer->getName()+"' has no object '"+mobject+"'." );
          }

          if ( ops->methods()->hasMember(mmethod) == false ) {
              //DumpObject( peer );
              if ( mobject != "this" )
                  throw parse_exception_no_such_method_on_component( "DataCall::"+mobject, mmethod );
              else
                  throw parse_exception_no_such_method_on_component( "DataCall::"+peer->getName(), mmethod );
          }
      }

      // create an argument parser for the call..
      // Store everything in the ArgumentsParser ! This DataCallParser instance is recursively called !
      ArgumentsParser* argspar =
          new ArgumentsParser( expressionparser, peer, ops,
                               mobject, mmethod );
      // we no longer need these two..
      mobject.clear();
      mmethod.clear();

      // keep hold of the argspar, we're still going to need it after
      // it's done its work..  ( in seendatacall(), that is.. )
      argparsers.push( argspar );

      // set the arguments parser to the parser provided by the
      // ArgumentsParser we just created..
      arguments = argspar->parser();
  }

  void DataCallParser::seendatacall()
  {
    ArgumentsParser* argspar = argparsers.top();
    argparsers.pop();
    std::string obj = argspar->objectname();
    std::string meth = argspar->methodname();
    std::vector<DataSourceBase::shared_ptr> args = argspar->result();
    OperationInterface* peer = argspar->object();
    delete argspar;

    // separate track if we are handling a constructor:
    if ( obj == "this" && TypeInfoRepository::Instance()->type( meth ) ) {
        ret = TypeInfoRepository::Instance()->type( meth )->construct( args );
        if (!ret) {
            throw parse_exception_no_such_constructor( meth, args );
        }
    } else {
        // plain method:

        OperationInterface* ops = peer;
        // we already checked for the existence of this object and method
        // in seendataname()..

        try {
            ret = ops->methods()->produce( meth, args );
        }
        catch( const wrong_number_of_args_exception& e )
            {
                throw parse_exception_wrong_number_of_arguments
                    (obj, meth, e.wanted, e.received );
            }
        catch( const wrong_types_of_args_exception& e )
            {
                throw parse_exception_wrong_type_of_argument
                    (obj, meth, e.whicharg, e.expected_, e.received_ );
            }
#ifndef NDEBUG
        catch(...) {
            assert(false);
        }
#endif
    }
    assert( ret.get() );
  }

  DataCallParser::~DataCallParser()
  {
    // if argparsers is not empty, then something went wrong during
    // the parsing ( someone threw an exception ), and we're
    // responsible for cleaning up the argparsers we created..
    while ( ! argparsers.empty() )
    {
      delete argparsers.top();
      argparsers.pop();
    };
  };

    error_status<> handle_no_value(scanner_t const& scan, parser_error<std::string, iter_t>& e )
    {
        //std::cerr << "No value in EP : "<<e.descriptor<<std::endl;
        // retry if it is a datacall, thus fail this rule
        return error_status<>( error_status<>::fail );
    }

    error_status<> handle_no_datacall(scanner_t const& scan, parser_error<std::string, iter_t>&e )
    {
        // retry with a member :
        //std::cerr << "No DataCall in EP : "<<e.descriptor<<std::endl;
        return error_status<>( error_status<>::fail );
    }

    error_status<> handle_no_member(scanner_t const& scan, parser_error<std::string, iter_t>&e )
    {
        // if this rule also fails, throw semantic exception ( member not found )
        throw parse_exception_semantic_error( e.descriptor );
        // dough ! not reached :
        return error_status<>( error_status<>::rethrow );
    }

  ExpressionParser::ExpressionParser( TaskContext* pc )
      : datacallparser( *this, pc ),
        valueparser( pc ),
        _invert_time(false),
        opreg( OperatorRepository::Instance() )
  {
    BOOST_SPIRIT_DEBUG_RULE( expression );
    BOOST_SPIRIT_DEBUG_RULE( unarynotexp );
    BOOST_SPIRIT_DEBUG_RULE( unaryminusexp );
    BOOST_SPIRIT_DEBUG_RULE( unaryplusexp );
    BOOST_SPIRIT_DEBUG_RULE( multexp );
    BOOST_SPIRIT_DEBUG_RULE( divexp );
    BOOST_SPIRIT_DEBUG_RULE( modexp );
    BOOST_SPIRIT_DEBUG_RULE( plusexp );
    BOOST_SPIRIT_DEBUG_RULE( minusexp );
    BOOST_SPIRIT_DEBUG_RULE( smallereqexp );
    BOOST_SPIRIT_DEBUG_RULE( smallerexp );
    BOOST_SPIRIT_DEBUG_RULE( greatereqexp );
    BOOST_SPIRIT_DEBUG_RULE( greaterexp );
    BOOST_SPIRIT_DEBUG_RULE( equalexp );
    BOOST_SPIRIT_DEBUG_RULE( notequalexp );
    BOOST_SPIRIT_DEBUG_RULE( orexp );
    BOOST_SPIRIT_DEBUG_RULE( andexp );
    BOOST_SPIRIT_DEBUG_RULE( ifthenelseexp );
    BOOST_SPIRIT_DEBUG_RULE( groupexp );
    BOOST_SPIRIT_DEBUG_RULE( dotexp );
    BOOST_SPIRIT_DEBUG_RULE( atomicexpression );
    BOOST_SPIRIT_DEBUG_RULE( time_expression );
    BOOST_SPIRIT_DEBUG_RULE( time_spec );
    BOOST_SPIRIT_DEBUG_RULE( indexexp );
    BOOST_SPIRIT_DEBUG_RULE( comma );
    BOOST_SPIRIT_DEBUG_RULE( close_brace );

    comma = expect_comma( ch_p(',') );
    close_brace = expect_close( ch_p(')') );
    expression = ifthenelseexp;

    // We parse expressions without regard of the types.  First we
    // parse the expressions, then we worry about whether what the
    // user says is actually valid.  You can try to add up two
    // booleans, and the parser will parse it, but it will notice
    // you're writing bogus when it tries to pass it to an operator
    // structure from Operators.hpp

    // TODO: implement the ifthenelse operator ?
    ifthenelseexp = andexp;
    andexp =
      orexp >> *( ( str_p( "&&" ) | "and" ) >> orexp[
                    bind( &ExpressionParser::seen_binary, this, "&&" ) ] );
    orexp =
      notequalexp >> *( ( str_p( "||" ) | "or" ) >> notequalexp[
                        bind( &ExpressionParser::seen_binary, this, "||" ) ] );
    notequalexp =
      equalexp >> *( "!=" >> equalexp[
                       bind( &ExpressionParser::seen_binary, this, "!=" ) ] );
    equalexp =
         greatereqexp
      >> *( "==" >> greatereqexp[
              bind( &ExpressionParser::seen_binary, this, "==" ) ] );
    greatereqexp =
         greaterexp
      >> *( ">=" >> greaterexp[
              bind( &ExpressionParser::seen_binary, this, ">=" ) ] );
    greaterexp =
         smallereqexp
      >> *( '>' >> smallereqexp[
              bind( &ExpressionParser::seen_binary, this, ">" ) ] );
    smallereqexp =
         smallerexp
      >> *( "<=" >> smallerexp[
              bind( &ExpressionParser::seen_binary, this, "<=" ) ] );
    smallerexp =
      minusexp >> *( '<' >> minusexp[
                       bind( &ExpressionParser::seen_binary, this, "<" ) ] );
    minusexp =
      plusexp >> *( '-' >> plusexp[
                      bind( &ExpressionParser::seen_binary, this, "-" ) ] );
    plusexp =
      modexp >> *( '+' >> modexp[
                     bind( &ExpressionParser::seen_binary, this, "+" ) ] );
    modexp =
      div_or_mul >> *( '%' >> div_or_mul[
                     bind( &ExpressionParser::seen_binary, this, "%" ) ] );
    div_or_mul =
      unaryplusexp >> *( ('/' >> unaryplusexp[
            bind( &ExpressionParser::seen_binary, this, "/" ) ] )
       | ('*' >> unaryplusexp[
            bind( &ExpressionParser::seen_binary, this, "*" ) ] ) );

    unaryplusexp =
        '+' >> unaryminusexp[
          bind( &ExpressionParser::seen_unary, this, "+" ) ]
      | unaryminusexp;
    unaryminusexp =
        '-' >> unarynotexp[
          bind( &ExpressionParser::seen_unary, this, "-" ) ]
      | unarynotexp;
    unarynotexp =
        ( str_p( "not" ) | '!' ) >> atomicexpression[
            bind( &ExpressionParser::seen_unary, this, "!" ) ]
        | atomicexpression;

    // note the order is important: commonparser.identifier throws a
    // useful "cannot use x as identifier" error if it fails, so we
    // must first show all non-identifier rules.
    atomicexpression = (
        // A parenthesis group.
      groupexp
        // or a time expression
      | time_expression
        // or a constant or user-defined value..
      | my_guard( valueparser.parser())[ &handle_no_value ]
               [ bind( &ExpressionParser::seenvalue, this ) ]
      | my_guard( datacallparser.parser() )[&handle_no_datacall]
                            [bind( &ExpressionParser::seendatacall, this ) ]
        // or an index or dot expression
        ) >> ! dotexp >> !indexexp;
    // take index of an atomicexpression
    indexexp =
        (ch_p('[') >> expression[bind(&ExpressionParser::seen_binary, this, "[]")] >> expect_close( ch_p( ']') ) );

    dotexp =
        +( ch_p('.') >> commonparser.identifier[ bind(&ExpressionParser::seen_dotmember, this, _1, _2)]);

    // needs no semantic action, its result is already on top of
    // the stack, where it should be..
    groupexp = '(' >> expression >> close_brace;

    // the day i find a clean way to temporarily disable 'eol' skipping, a lot of
    // grammar will look better...
    time_expression =
        (str_p("time")>>eps_p(~commonparser.identchar | eol_p | end_p ))[bind(&ExpressionParser::seentimeexpr, this)]
        |
        ( (eps_p[boost::lambda::var(eol_skip_functor::skipeol) = false] >> uint_p[ bind( &ExpressionParser::seentimespec, this, _1 ) ]
           >> (str_p( "s" ) | "ms" | "us" | "ns" )[boost::lambda::var(eol_skip_functor::skipeol) = true][bind( &ExpressionParser::seentimeunit, this, _1, _2 ) ])
          | (eps_p[boost::lambda::var(eol_skip_functor::skipeol) = true] >> nothing_p) // eps_p succeeds always, then fail.
          )
          ; // enable skipeol.

//         >> expect_timespec( (( str_p( ">=" ) | ">" )
//                             |
//                             (str_p("<=") | "<")[bind( &ExpressionParser::inverttime, this)])
//         >> time_spec);

//     time_spec =
//         ( uint_p[ bind( &ExpressionParser::seentimespec, this, _1 ) ]
//         >>
//       ( str_p( "s" ) | "ms" | "us" | "ns" )[
//         bind( &ExpressionParser::seentimeunit, this, _1, _2 ) ] ) | expression[bind(&ExpressionParser::seentimeexpr, this)];

  };

    void ExpressionParser::inverttime()
    {
        _invert_time = true;
    }

    void ExpressionParser::seentimeexpr()
    {
        parsestack.push( new DataSourceTime() );

//         DataSourceBase::shared_ptr res = parsestack.top();
//         parsestack.pop();
//         DataSource<double>::shared_ptr dres = dynamic_cast<DataSource<double>*>( res.get() );
//         if ( !dres )
//             throw parse_exception_semantic_error("Expected time in seconds but expression is not a floating point number.");
//         DataSourceBase::shared_ptr dsb( new DataSourceCondition( new ConditionDSDuration( dres, _invert_time ) ) );
//         _invert_time = false;
//         parsestack.push( dsb );
    }

  void ExpressionParser::seentimeunit( iter_t begin, iter_t end)
  {
    // the string starting at begin, ending at end is either ms, us,
    // ns or s, so we only need to check the first letter...
      // Convert to seconds...
      TimeService::Seconds total = 0;
    switch( *begin )
    {
    case 's': total = tsecs;
        break;
    case 'm': total = tsecs / 1000.0;
        break;
    case 'u': total = tsecs / 1000000.0;
        break;
    case 'n': total = tsecs / 1000000000.0;
        break;
    default:
        std::string arg(begin, end);
        throw parse_exception_semantic_error("Expected time expression 's', 'ms', 'us' or 'ns' after integer value, got "+arg);
    };

    parsestack.push( new ConstantDataSource<double>( total ) );

//     DataSourceBase::shared_ptr dsb( new DataSourceCondition(
//       new ConditionDuration( total, _invert_time ) ) );
//     _invert_time = false;
//     parsestack.push( dsb );
  }

  void ExpressionParser::seentimespec( int n )
  {
    tsecs = n;
  }

  void ExpressionParser::seenvalue()
  {
    DataSourceBase::shared_ptr ds = valueparser.lastParsed();
    parsestack.push( ds );
  }

  void ExpressionParser::seendatacall()
  {
      DataSourceBase::shared_ptr n( datacallparser.getParseResult() );
      parsestack.push( n );
  }

  ExpressionParser::~ExpressionParser()
  {
      // if parsestack is not empty, then something went wrong, someone
      // threw an exception, so we clean up..
      while ( !parsestack.empty() )
          parsestack.pop();
  }

  rule_t& ExpressionParser::parser()
  {
    return expression;
  }

  DataSourceBase::shared_ptr ExpressionParser::getResult()
  {
    assert( !parsestack.empty() );
    return parsestack.top();
  }

  void ExpressionParser::seen_unary( const std::string& op )
  {
    DataSourceBase::shared_ptr arg( parsestack.top() );
    parsestack.pop();
    DataSourceBase::shared_ptr ret =
        opreg->applyUnary( op, arg.get() );
    if ( ! ret )
        throw parse_exception_fatal_semantic_error( "Cannot apply unary operator \"" + op +
                                                    "\" to " + arg->getType() +"." );
    parsestack.push( ret );
  };

  void ExpressionParser::seen_dotmember( iter_t s, iter_t f )
  {
      std::string member(s,f);
      // inspirired on seen_unary
    DataSourceBase::shared_ptr arg( parsestack.top() );
    parsestack.pop();
    DataSourceBase::shared_ptr ret =
      opreg->applyDot( member, arg.get() );
    if ( ! ret )
      throw parse_exception_fatal_semantic_error( arg->getType() + " does not have member \"" + member +
                                            "\"." );
    parsestack.push( ret );
  };

  void ExpressionParser::seen_binary( const std::string& op )
  {
    DataSourceBase::shared_ptr arg1( parsestack.top() );
    parsestack.pop();
    DataSourceBase::shared_ptr arg2( parsestack.top() );
    parsestack.pop();

    // Arg2 is the first (!) argument, as it was pushed on the stack
    // first.
    DataSourceBase::shared_ptr ret =
      opreg->applyBinary( op, arg2.get(), arg1.get() );
    if ( ! ret )
      throw parse_exception_fatal_semantic_error( "Cannot apply binary operation "+ arg2->getType() +" " + op +
                                            " "+arg1->getType() +"." );
    parsestack.push( ret );
  };

  void ExpressionParser::dropResult()
  {
    parsestack.pop();
  }
}
