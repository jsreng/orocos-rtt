
#include "execution/FunctionGraphBuilder.hpp"
#include "execution/CommandComposite.hpp"
#include "execution/TaskAttribute.hpp"
#include "execution/TemplateFactories.hpp"
//#include "execution/parse_exception.hpp"
#include "execution/FactoryExceptions.hpp"
#include "GraphCopier.hpp"

#include "corelib/CommandNOP.hpp"
#include "corelib/ConditionFalse.hpp"
#include "corelib/ConditionTrue.hpp"
#include <boost/graph/copy.hpp>
#include <utility>

#include <iostream>

namespace ORO_Execution
{
    /**
     * @todo 
     * 1. Refactor this file such that ProgramGraph becomes FunctionGraphBuilder, thus it becomes a pure factory.
     * 2. add copy semantics to ProgramInterface, such that FunctionGraph::copy can be called from the interface.
     */

    using namespace boost;
    using namespace std;
    using ORO_CoreLib::CommandNOP;
    using ORO_CoreLib::ConditionTrue;

    FunctionGraphBuilder::FunctionGraphBuilder()
        : graph( 0 )
    {
    }

    FunctionGraphBuilder::~FunctionGraphBuilder()
    {
    }

    FunctionGraphPtr FunctionGraphBuilder::startFunction(const std::string& fname)
    {
        // next node should be 'empty'/ not used here.
        // a function is to be constructed
        func.reset( new FunctionGraph( fname ) );
        graph   = &func->getGraph();
        build   = func->startNode();
        next    = add_vertex( *graph );
        put( vertex_exec, *graph, next, VertexNode::normal_node );
        return func;
    }

    void FunctionGraphBuilder::returnFunction( ConditionInterface* cond, int line )
    {
        // connect the build node to the exitNode under a condition,
        // for example, the build implicit term condition.
        add_edge(build, func->exitNode(), EdgeCondition(cond), *graph);
    }

    FunctionGraphPtr FunctionGraphBuilder::getFunction()
    {
        return func;
    }

    FunctionGraphPtr FunctionGraphBuilder::endFunction( int endline )
    {
        // the map contains _references_ to all vertex_command properties
        boost::property_map<Graph, vertex_command_t>::type
            cmap = get(vertex_command, *graph);
        cmap[func->exitNode()].setLineNumber( endline );

        // A return statement is obligatory, so the returnFunction has
        // already connected the build node to exitNode of the function.
        // the end of the statement will proceedToNext
        // remove the empty next nodes.
        remove_vertex( build, *graph );
        remove_vertex( next, *graph );

        func->finish();
        func->reset();

        FunctionGraphPtr tfunc = func;
        func.reset();
        return tfunc;
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::addCommand( ConditionInterface* cond,  CommandInterface* com )
    {
        add_edge(build, next, EdgeCondition(cond), *graph);
        build = next;
        setCommand(com);
        next    = add_vertex( *graph );
        put( vertex_exec, *graph, next, VertexNode::normal_node );
        return next;
    }

    void FunctionGraphBuilder::addConditionEdge( ConditionInterface* cond, CommandNode vert )
    {
        add_edge(build, vert, EdgeCondition(cond), *graph);
    }

    void FunctionGraphBuilder::closeConditionEdge( CommandNode vert, ConditionInterface* cond )
    {
        add_edge(vert, build, EdgeCondition(cond), *graph);
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::moveTo( CommandNode _build, CommandNode _next, int linenumber )
    {
        this->setLineNumber( linenumber );
        CommandNode old = build;
        build = _build;
        next    = _next;
        return old;
    }

    CommandInterface* FunctionGraphBuilder::getCommand( CommandNode cn )
    {
        // the map contains _references_ to all vertex_command properties
        boost::property_map<Graph, vertex_command_t>::type
            cmap = get(vertex_command, *graph);
        // access the one of build
        return cmap[cn].getCommand();
    }

    void FunctionGraphBuilder::setCommand( CommandInterface* comm )
    {
        this->setCommand(build, comm);
    }

    void FunctionGraphBuilder::setCommand(CommandNode cn, CommandInterface* comm )
    {
        // the map contains _references_ to all vertex_command properties
        boost::property_map<Graph, vertex_command_t>::type
            cmap = get(vertex_command, *graph);
        // access the one of build
        delete cmap[cn].setCommand( comm );
    }

    void FunctionGraphBuilder::setName(const std::string& pname) {
        func->setName(pname);
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::proceedToNext( ConditionInterface* cond, int this_line )
    {
        add_edge(build, next, EdgeCondition(cond), *graph);
        return proceedToNext( this_line );
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::proceedToNext( int this_line )
    {
        if ( this_line )
            this->setLineNumber( this_line );
        build = next;
        next    = add_vertex( *graph );
        put(vertex_exec, *graph, next, VertexNode::normal_node );
        return build;
    }

    void FunctionGraphBuilder::connectToNext( CommandNode v, ConditionInterface* cond )
    {
        add_edge( v, next, EdgeCondition(cond), *graph);
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::buildNode() const
    {
        return build;
    }

    size_t FunctionGraphBuilder::buildEdges() const
    {
        return out_degree( build, *graph );
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::nextNode() const
    {
        return next;
    }

    void FunctionGraphBuilder::setLineNumber( int line )
    {
        boost::property_map<Graph, vertex_command_t>::type
            cmap = get(vertex_command, *graph);
        cmap[build].setLineNumber( line );
    }


    template< class _Map >
    struct finder {
        typedef FunctionGraphBuilder::Vertex first_argument_type ;
        typedef std::pair<_Map, int> second_argument_type ;
        typedef bool result_type;
        bool operator()(const FunctionGraphBuilder::Vertex& v,
                        const std::pair<_Map, int>& to_find  ) const
            {
                // return : Is this the node with the given property ?
                return to_find.first[v] == to_find.second;
            }
    };

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::appendFunction( ConditionInterface* cond, FunctionGraphPtr fn,
                                                            std::vector<DataSourceBase::shared_ptr> fnargs )
    {
        /**
         * This can be used if a function must be called conditionally.
         */
        // Do not move build inhere !
        // copy FunctionGraph's graph into the current *graph, add an edge with
        // the condition. Connect the exitNode() of the function with
        // the next node.

        // The reason to reindex the vertexes of source graph is
        // to silence valgrinds warnings about uninitialised values
        // in the following piece of code. Technically, copy_graph
        // does not need it.
        boost::property_map<Graph, vertex_index_t>::type indexmap =
            get( vertex_index, *graph );
        boost::graph_traits<Graph>::vertex_iterator v, vend;
        int index = 0;
        for ( tie( v, vend ) = vertices( *graph ); v != vend; ++v )
            indexmap[*v] = index++;

        // check nb of arguments:
        std::vector<TaskAttributeBase*> origlist = fn->getArguments();
        if ( fnargs.size() != origlist.size() )
            throw wrong_number_of_args_exception( origlist.size(), fnargs.size() );
            
        // make a deep copy of the function :
        std::map<const DataSourceBase*, DataSourceBase*> replacementdss;
        std::vector<TaskAttributeBase*> newlist;
        for (unsigned int i=0; i < origlist.size(); ++i)
            newlist.push_back( origlist[i]->copy( replacementdss, false ) ); // args are not instantiated.
        // newlist contains the DS which need initialisations :

        // create commands that init all the args :
        CommandComposite* icom=  new CommandComposite();
        std::vector<DataSourceBase::shared_ptr>::const_iterator dit = fnargs.begin();
        std::vector<TaskAttributeBase*>::const_iterator tit =  newlist.begin();
        try {
            for (; dit != fnargs.end(); ++dit, ++tit)
                icom->add( (*tit)->assignCommand( *dit, true ) );
        }
        catch( const bad_assignment& e) {
            // cleanup allocated memory
            for (unsigned int i=0; i < newlist.size(); ++i)
                delete newlist[i];
            delete icom;
            int parnb = (dit - fnargs.begin());
            throw wrong_types_of_args_exception(parnb, (*tit)->toDataSource()->getType() ,(*dit)->getType() );
        }

        // set the init command on the build node 
        //assert( build not used by other than NOP )
        assert( dynamic_cast<CommandNOP*>( this->getCommand(build) ));
        this->setCommand( icom );

        boost::copy_graph( fn->getGraph(), *graph,
                           boost::vertex_copy( GraphVertexCopier( fn->getGraph(), *graph, replacementdss ) ).
                           edge_copy( GraphEdgeCopier( fn->getGraph(), *graph, replacementdss ) ) );

        // cleanup newlist, the (var)DS's are stored in the assignCommand
        for (unsigned int i=0; i < newlist.size(); ++i)
            delete newlist[i];

        // the subgraph has been copied but is now 'floating' in the current graph.
        // we search func start and exit points and connect them to
        // the current graph.

        // domi: it would be cleaner if a function would keep a
        // reference to its enter and exit point, and we would use the
        // copy_graph orig_to_copy map to find the corresponding nodes
        // in the copy...
        graph_traits<Graph>::vertex_iterator v1,v2, vc;
        tie(v1,v2) = vertices(*graph);
        boost::property_map<Graph, vertex_exec_t>::type
            vmap = get(vertex_exec, *graph);

//         // test not initialised value.
//         vc = v1;
//         while (vc != v2) {
//             std::cerr << int( vmap[*vc] )<<std::endl;
//             ++vc;
//         }
//         std::cerr<<std::endl;
//         // test foo :
//         tie(v1,v2) = vertices(fn->getGraph());
//             vmap = get(vertex_exec, fn->getGraph());
//         vc = v1;
//         while (vc != v2) {
//             std::cerr << int( vmap[*vc] )<<std::endl;
//             ++vc;
//         }


        Vertex funcStart=*
                         find_if(v1, v2,
                                 bind2nd( finder<boost::property_map<Graph, vertex_exec_t>::type>() ,
                                          std::make_pair( vmap, int(VertexNode::func_start_node)) ) );
        Vertex funcExit=*
                        find_if(v1, v2,
                                bind2nd( finder<boost::property_map<Graph, vertex_exec_t>::type>() ,
                                         std::make_pair( vmap, int(VertexNode::func_exit_node)) ) );

        // reset their special meanings.
        vmap[funcStart] = VertexNode::normal_node;
        vmap[funcExit]  = VertexNode::normal_node;
        // connect the graph.
        addConditionEdge( cond, funcStart );
        connectToNext( funcExit, new ConditionTrue );

        return funcExit;
//         // try to solve call in function bug with arguments
//         boost::property_map<Graph, vertex_command_t>::type
//             cmap = get(vertex_command, *graph);
//         return proceedToNext( cmap[funcExit].getLineNumber() );
    }

    FunctionGraphBuilder::CommandNode FunctionGraphBuilder::setFunction( FunctionGraphPtr fn,
                                                         std::vector<DataSourceBase::shared_ptr> fnargs )
    {
        /**
         * This function must/should be used when build has no edges yet leaving.
         * Off course, edges can already be pointing to build of the previous
         * command. That's why build can not be overwritten with the first node
         * of the function, and we need to insert an extra edge from build to
         * funcStart. That is why we use (until better solution found) the append
         * Function.
         */
        return appendFunction( new ConditionTrue, fn, fnargs) ;
    }

    void FunctionGraphBuilder::startIfStatement( ConditionInterface* cond, int linenumber )
    {
        // push all relevant nodes on the branch_stack.
        // endIf and endElse will pop them

        // * next will become the first node of the succeeding if statement
        // * after_else_node will become the node after the if block is finished
        // and the else block is finished.
        // * else_node is the first node of a failing if statement (this may be _any_statement, thus also an if)
        CommandNode else_node = add_vertex( *graph );
        put(vertex_exec, *graph, else_node, VertexNode::normal_node );
        branch_stack.push( else_node );
        CommandNode after_else_node =  add_vertex( *graph );
        put(vertex_exec, *graph, after_else_node, VertexNode::normal_node );
        branch_stack.push( after_else_node );
        //branch_stack.push( build );

        // add edge from build to next
        addConditionEdge( cond, next );
        // add edge from build to 'after_else_node'
        addConditionEdge( new ConditionTrue(), else_node );
        proceedToNext(linenumber);
    }

    void FunctionGraphBuilder::endIfBlock(int linenumber){
        // this is called after a proceedToNext of the last statement of
        // the if block.
        // Connect end of if block with after_else_node
        CommandNode after_else_node = branch_stack.top();
        addConditionEdge( new ConditionTrue(), after_else_node );
        branch_stack.pop();
        // make else_node build, next remains.
        moveTo( branch_stack.top(), next, linenumber );
        branch_stack.pop();
        // store again !
        branch_stack.push( after_else_node );
    }

    // Else : can be empty and is then a plain proceed to next.
    void FunctionGraphBuilder::endElseBlock(int linenumber) {
        // after_else_node is on top of stack
        CommandNode after_else_node = branch_stack.top();
        branch_stack.pop();
        addConditionEdge( new ConditionTrue(), after_else_node );
        // make after_else_node build
        moveTo( after_else_node, next, linenumber );
    }

    void FunctionGraphBuilder::startWhileStatement( ConditionInterface* cond, int linenumber )
    {
        // very analogous to the if statement, but there is no else part
        // and we stack the first commandnode to be able to close the loop.
        CommandNode after_while_node = add_vertex( *graph );
        put(vertex_exec, *graph, after_while_node, VertexNode::normal_node );
        branch_stack.push( after_while_node );
        break_stack.push( after_while_node );
        branch_stack.push( build );
        // add edge from build to next if condition == true
        addConditionEdge( cond, next );
        // if condition fails, go from build to 'after_while_node'
        addConditionEdge( new ConditionTrue(), after_while_node );
        proceedToNext(linenumber);
    }

    void FunctionGraphBuilder::endWhileBlock(int linenumber)
    {
        CommandNode start_of_while = branch_stack.top();
        branch_stack.pop();
        // go from build back to start (and check there the condition).
        addConditionEdge( new ConditionTrue(), start_of_while );
        CommandNode after_while_node =  branch_stack.top();
        branch_stack.pop();
        break_stack.pop();
        moveTo( after_while_node, next, linenumber );
    }

    bool FunctionGraphBuilder::inLoop()
    {
        return break_stack.size() != 0;
    }

    bool FunctionGraphBuilder::breakLoop()
    {
        if ( !inLoop() )
            return false;

        // go from build to last nested exit point.
        addConditionEdge( new ConditionTrue(), break_stack.top() );
        return true;
    }

#if 0
    void FunctionGraphBuilder::prependCommand( CommandInterface* command, int line_nr )
    {
        CommandNode previousstart = start;
        start = add_vertex( program );
        boost::property_map<Graph, vertex_exec_t>::type
            vmap = get(vertex_exec, program);
        put( vmap, start, get( vmap, previousstart ) );
        put( vmap, previousstart, VertexNode::normal_node );
        boost::property_map<Graph, vertex_command_t>::type
            cmap = get( vertex_command, program );
        VertexNode vnode( command );
        vnode.setLineNumber( line_nr );
        put( cmap, start, vnode );
        add_edge(start, previousstart, EdgeCondition(new ConditionTrue), program);
        if ( build == previousstart )
            build = start;

        // now let's reindex the vertices...
        boost::property_map<Graph, vertex_index_t>::type indexmap =
            get( vertex_index, *graph );
        boost::graph_traits<Graph>::vertex_iterator v, vend;
        int index = 0;
        for ( tie( v, vend ) = vertices( *graph ); v != vend; ++v )
            indexmap[*v] = index++;
    }
#endif

}
