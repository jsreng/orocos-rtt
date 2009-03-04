
#ifndef ORO_LOCAL_MESSAGE_HPP
#define ORO_LOCAL_MESSAGE_HPP

#include <boost/function.hpp>
#include <string>
#include "Invoker.hpp"
#include "MessageBase.hpp"
#include "MessageStorage.hpp"
#include "MessageProcessor.hpp"

namespace RTT
{
    namespace detail
    {
        template<class FunctionT>
        class LocalMessageImpl
            : public MessageBase<FunctionT>
        {
        protected:
            MessageStorage<FunctionT> mmesg;
            MessageProcessor* mmp;
        public:
            typedef FunctionT Signature;
            typedef boost::function_traits<Signature> traits;
            typedef bool result_type;

            bool execute() {
                mmesg.execute();
                return true;
            }

            void readArguments() {}

            /**
             * Invoke this operator if the message has no arguments.
             */
            result_type invoke()
            {
                return mmp->process( mmesg.rtclone() );
            }

            /**
             * Invoke this operator if the message has one argument.
             */
            template<class T1>
            result_type invoke(T1 t)
            {
                MessageStorage<FunctionT>* c = mmesg.rtclone();
                c->store(t);
                return mmp->process( c );
            }

            /**
             * Invoke this operator if the message has two arguments.
             */
            template<class T1, class T2>
            result_type invoke(T1 t1, T2 t2)
            {
                MessageStorage<FunctionT>* c = mmesg.rtclone();
                c->store(t1,t2);
                return mmp->process( c );
            }

            /**
             * Invoke this operator if the message has three arguments.
             */
            template<class T1, class T2, class T3>
            result_type invoke(T1 t1, T2 t2, T3 t3)
            {
                MessageStorage<FunctionT>* c = mmesg.rtclone();
                c->store(t1,t2,t3);
                return mmp->process( c );
            }

            /**
             * Invoke this operator if the message has four arguments.
             */
            template<class T1, class T2, class T3, class T4>
            result_type invoke(T1 t1, T2 t2, T3 t3, T4 t4)
            {
                MessageStorage<FunctionT>* c = mmesg.rtclone();
                c->store(t1,t2,t3,t4);
                return mmp->process( c );
            }

        };

        /**
         * A message which executes a local function.
         *
         * Usage:
         @code
         LocalMessage<double(int, double)> mymeth( &Class::foo, &c);
         double result = mymeth( 3, 1.9);
         @endcode
        */
        template<class FunctionT>
        struct LocalMessage
            : public Invoker<FunctionT,LocalMessageImpl<FunctionT>, bool >
        {
            typedef FunctionT Signature;
            typedef bool result_type;
            typedef boost::function_traits<Signature> traits;

            /**
             * Construct a LocalMessage from a member function pointer
             * and an object derived from TaskContext.
             *
             * @param mesg A pointer to a class member function
             * @param object An object of the class which has \a mesg as member function
             * and which is a subclass of TaskContext.
             */
            template<class M, class ObjectType>
            LocalMessage(M mesg, ObjectType object)
            {
                this->mmesg.setup(mesg, object);
                this->mmp = object->engine()->messages();
            }

            /**
             * Construct a LocalMessage from a function pointer or function object.
             *
             * @param mesg a pointer to a function or function object.
             * @param mp The MessageProcessor in which this message must be executed.
             */
            template<class M>
            LocalMessage(M mesg, MessageProcessor* mp)
            {
                this->mmesg.setup(mesg);
                this->mmp = mp;
            }

            boost::function<Signature> getMessageFunction() const
            {
                return this->function();
            }

            RTT::ActionInterface* clone() const {
                return new LocalMessage(*this);
            }

        };
    }
}

#endif
