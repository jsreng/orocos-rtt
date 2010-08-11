#ifndef ORO_INVOKER_BASE_HPP
#define ORO_INVOKER_BASE_HPP

#include <boost/function.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/type_traits.hpp>
#include "NA.hpp"
#include "../SendHandle.hpp"
#include "../rtt-fwd.hpp"

namespace RTT
{
    namespace internal
    {
        /**
         * A class which converts a function type signature \a F
         * to a virtual send()/call() interface. The integer
         * is the number of arguments of F.
         */
        template<int, class F>
        struct InvokerBaseImpl;

        /**
         * This is the base class that defines the interface
         * of all invocable method implementations.
         * Any invocable method implementation must inherit
         * from this class such that it can be used transparantly
         * by the Method, Operation and SendHandle containers.
         */
        template<class F>
        struct InvokerBase
            : public InvokerBaseImpl<boost::function_traits<F>::arity, F>
        {};

        template<class F>
        struct InvokerBaseImpl<0,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function_traits<F>::result_type result_reference;
            virtual ~InvokerBaseImpl() {}
            virtual SendHandle<F> send() = 0;
            virtual result_type call() = 0;
        };

        template<class F>
        struct InvokerBaseImpl<1,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1) = 0;
            virtual SendHandle<F> send(arg1_type a1) = 0;
        };

        template<class F>
        struct InvokerBaseImpl<2,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            typedef typename boost::function<F>::arg2_type arg2_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1, arg2_type a2) = 0;
            virtual SendHandle<F> send(arg1_type a1, arg2_type a2) = 0;
        };

        template<class F>
        struct InvokerBaseImpl<3,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            typedef typename boost::function<F>::arg2_type arg2_type;
            typedef typename boost::function<F>::arg3_type arg3_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1, arg2_type a2, arg3_type a3) = 0;
            virtual SendHandle<F> send(arg1_type a1, arg2_type a2, arg3_type a3) = 0;
        };

        template<class F>
        struct InvokerBaseImpl<4,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            typedef typename boost::function<F>::arg2_type arg2_type;
            typedef typename boost::function<F>::arg3_type arg3_type;
            typedef typename boost::function<F>::arg4_type arg4_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) = 0;
            virtual SendHandle<F> send(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) = 0;
        };

        template<class F>
        struct InvokerBaseImpl<5,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            typedef typename boost::function<F>::arg2_type arg2_type;
            typedef typename boost::function<F>::arg3_type arg3_type;
            typedef typename boost::function<F>::arg4_type arg4_type;
            typedef typename boost::function<F>::arg5_type arg5_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) = 0;
            virtual SendHandle<F> send(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) = 0;
        };

        template<class F>
        struct InvokerBaseImpl<6,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            typedef typename boost::function<F>::arg2_type arg2_type;
            typedef typename boost::function<F>::arg3_type arg3_type;
            typedef typename boost::function<F>::arg4_type arg4_type;
            typedef typename boost::function<F>::arg5_type arg5_type;
            typedef typename boost::function<F>::arg6_type arg6_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6) = 0;
            virtual SendHandle<F> send(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6) = 0;
        };

        template<class F>
        struct InvokerBaseImpl<7,F>
        {
            typedef typename boost::function_traits<F>::result_type result_type;
            typedef typename boost::function<F>::arg1_type arg1_type;
            typedef typename boost::function<F>::arg2_type arg2_type;
            typedef typename boost::function<F>::arg3_type arg3_type;
            typedef typename boost::function<F>::arg4_type arg4_type;
            typedef typename boost::function<F>::arg5_type arg5_type;
            typedef typename boost::function<F>::arg6_type arg6_type;
            typedef typename boost::function<F>::arg7_type arg7_type;
            virtual ~InvokerBaseImpl() {}
            virtual result_type call(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6, arg7_type a7) = 0;
            virtual SendHandle<F> send(arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5, arg6_type a6, arg7_type a7) = 0;
        };
   }
}
#endif
