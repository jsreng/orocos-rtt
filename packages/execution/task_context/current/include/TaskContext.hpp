#ifndef ORO_TASK_CONTEXT_HPP
#define ORO_TASK_CONTEXT_HPP

#include <os/Mutex.hpp>

#include "Factories.hpp"
#include "AttributeRepository.hpp"

#include <string>
#include <map>

namespace ORO_Execution
{
    class Processor;

    /**
     * A TaskContext groups the operations, events, datasources,
     * peer-tasks and processor a task has. It links related tasks
     * and allows to iterate over its peers.
     *
     * When a peer is added, the (script) programs of this task can access
     * the peer using peername.methodname() or peername.objname.methodname().
     * Peer commands should always be executed by the peer's Processor.
     *
     * It is assumed that the RunnableInterface of the TaskInterface
     * instance of this TaskContext invokes
     * the Processor::doStep() method in its RunnableInterface::step() method.
     * In this way, the user of this class can determine himself at which
     * point and at which moment remote commands and local programs can be executed.
     */
    class TaskContext
    {
        Processor*     _task_proc;
        bool           _task_proc_owner;
        std::string    _task_name;
    
        typedef std::map< std::string, TaskContext* > PeerMap;
        PeerMap         _task_map;

        ORO_OS::Mutex execguard;
    public:
        typedef std::vector< std::string > PeerList;

        /**
         * Create a TaskContext visible with \a name, which commands are processed
         * by \a proc. If no Processor is given, a default one is constructed.
         */
        TaskContext(const std::string& name, Processor* proc = 0 );

        ~TaskContext();

        /**
         * Execute or queue a command.
         * If processor->getTask() && processor->getTask()->isRunning(), the 
         * command \a c will be queued and executed by the processor, otherwise,
         * the command is executed directly.
         */
        bool executeCommand( CommandInterface* c);

        /**
         * Queue a command. If the Processor is not running,
         * this will fail and return zero.
         * @return The command id the processor returned.
         */
        int queueCommand( CommandInterface* c);

        /**
         * Get the name of this TaskContext.
         */
        const std::string& getName()
        {
            return _task_name;
        }

        /**
         * Change the name of this TaskContext.
         */
        void setName(const std::string& n)
        {
            _task_name = n;
        }
        /**
         * Add a one-way connection from this task to a peer task.
         * @param peer The peer to add.
         * @param alias An optional alias (another name) for the peer.
         * defaults to \a peer->getName()
         */
        bool addPeer( TaskContext* peer, std::string alias = "" );

        /**
         * Remove a one-way connection from this task to a peer task.
         */
        void removePeer( const std::string& name );

        /**
         * Add a two-way connection from  this task to a peer task.
         * This method is strict : both peers must not be connected to succeed.
         */
        bool connectPeers( TaskContext* peer );

        /**
         * Remove a two-way connection from this task to a peer task.
         * This method is strict : both peers must be connected to succeed.
         */
        void disconnectPeers( const std::string& name );

        /**
         * Return a standard container which contains all the Peer's names
         */
        PeerList getPeerList() const;

        /**
         * Return true if it knows a peer by that name.
         */
        bool hasPeer( const std::string& peer_name ) const;

        TaskContext* getPeer(const std::string& peer_name ) const;

        /**
         * Returns the processor of our task.
         */
        Processor* getProcessor() const
        {
            return _task_proc;
        }

        void debug(bool);

        /**
         * The commandFactory of this TaskContext.
         */
        GlobalCommandFactory    commandFactory;
        GlobalDataSourceFactory dataFactory;
        GlobalMethodFactory     methodFactory;

        /**
         * The task-local values ( attributes ) of this TaskContext.
         */
        AttributeRepository         attributeRepository;
    };
}

#endif
