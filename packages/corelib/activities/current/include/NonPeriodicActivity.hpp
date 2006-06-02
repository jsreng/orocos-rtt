/***************************************************************************
  tag: Peter Soetens  Tue Apr 5 16:53:25 CEST 2005  NonPeriodicActivity.hpp 

                        NonPeriodicActivity.hpp -  description
                           -------------------
    begin                : Tue April 05 2005
    copyright            : (C) 2005 Peter Soetens
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
 
 
#ifndef TASK_NON_PERIODIC_HPP
#define TASK_NON_PERIODIC_HPP

#include "ActivityInterface.hpp"
#include "RunnableInterface.hpp"
#include "EventProcessor.hpp"
#include "os/SingleThread.hpp"

#ifdef ORO_PRAGMA_INTERFACE
#pragma interface
#endif

namespace ORO_CoreLib
{

    /**
     * @brief A ActivityInterface implementation which is run in a SingleThread.
     *
     * By default, (if no RunnableInterface is given and no overloads
     * are present ) \a loop() will block for Events to be processed and
     * can be stopped by calling stop(). To check for events from your overloads or the RunnableInterface,
     * call this->processor()->step(), for non blocking checks and this->processor()->loop() for blocking
     * on Events.
     */
    class NonPeriodicActivity
        :public ActivityInterface,
         private ORO_OS::SingleThread
    {
    public:
        using ORO_OS::SingleThread::run;

        /**
         * Create an NonPeriodicActivity with a given priority and
         * RunnableInterface instance.
         * @param priority The priority of the underlying thread.
         * @param _r The optional runner, if none, this->loop() is called.
         */
        NonPeriodicActivity(int priority, RunnableInterface* _r = 0 );

        /**
         * Create an NonPeriodicActivity with a given priority, name and
         * RunnableInterface instance.
         * @param priority The priority of the underlying thread.
         * @param name The name of the underlying thread.
         * @param _r The optional runner, if none, this->loop() is called.
         */
        NonPeriodicActivity(int priority, const std::string& name, RunnableInterface* _r = 0 );

        virtual ~NonPeriodicActivity();

        /**
         * Run another (or self in case of null)
         * task.
         */
        bool run( RunnableInterface* r );

        /**
         * Run another (or self in case of null)
         * task. This overload is needed to redirect
         * the call of run( 0 ) to the correct function.
         */
        bool run( int ) { RunnableInterface* d(0); return NonPeriodicActivity::run( d ); }

        virtual Seconds getPeriod() const ;

        virtual EventProcessor* getEventProcessor() const ;

        virtual ThreadInterface* thread();

        virtual bool initialize();

        virtual void loop();

        virtual bool breakLoop();

        virtual void finalize();

        virtual bool isPeriodic() const;

        virtual bool start();

        virtual bool trigger();

        virtual bool stop();

        virtual bool isRunning() const;

        virtual bool isActive() const;

    private:
        BlockingEventProcessor* proc;
        RunnableInterface*      runner;
};

}


#endif