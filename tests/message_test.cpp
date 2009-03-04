#include "message_test.hpp"
#include <unistd.h>
#include <iostream>
#include <Message.hpp>
#include <OperationInterface.hpp>
#include <SequentialActivity.hpp>

#include <boost/function_types/function_type_signature.hpp>

#include <rtt-config.h>

using namespace std;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( MessageTest );

class MessageTask: public TaskContext
{
public:
    MessageTask() : TaskContext("root") {}

    // unusual signatures
    double m1r(double& a) { a = 2*a; return a; }
    double m1cr(const double& a) { return a; }

    // plain argument tests:
    void m0() { return ; }
    void m1(int i) { return ; }
    void m2(int i, double d) { return ; }
    void m3(int i, double d, bool c) { return ; }
    void m4(int i, double d, bool c, std::string s) { return ; }

    bool assertBool( bool b) { return b;}
};

void
MessageTest::setUp()
{
    tc =  new MessageTask();
    mp = new MessageProcessor();
    act = new SequentialActivity(mp);
    tc->addObject( this->createMessageFactory() );
    CPPUNIT_ASSERT(tc->start());
    CPPUNIT_ASSERT(act->start());
}


void
MessageTest::tearDown()
{
    tc->stop();
    act->stop();
    delete tc;
    delete mp;
    delete act;
}

TaskObject* MessageTest::createMessageFactory()
{
    TaskObject* to = new TaskObject("messages");
#if 0
    to->methods()->addMethod( method("assert",  &MessageTask::assertBool, tc), "assert","b","bd");

    to->messages()->addMessage( message("m0",  &MessageTask::m0, tc), "M0");
    to->messages()->addMessage( message("m1",  &MessageTask::m1, tc), "M1","a","ad");
    to->messages()->addMessage( message("m2",  &MessageTask::m2, tc), "M2","a","ad","a","ad");
    to->messages()->addMessage( message("m3",  &MessageTask::m3, tc), "M3","a","ad","a","ad","a","ad");
    to->messages()->addMessage( message("m4",  &MessageTask::m4, tc), "M4","a","ad","a","ad","a","ad","a","ad");
#endif
    return to;
}

void MessageTest::testMessage()
{
    Message<void(void)> m0("m0", boost::bind(&MessageTask::m0, tc), mp);
#if 0
    Message<void(int)> m1("m1", boost::bind(&MessageTask::m1, tc, _1), mp);
    Message<void(int,double)> m2("m2", boost::bind(&MessageTask::m2, tc,_1,_2), mp);
    Message<void(int,double,bool)> m3("m3", boost::bind(&MessageTask::m3, tc,_1,_2,_3), mp);
    Message<void(int,double,bool,std::string)> m4("m4", boost::bind(&MessageTask::m4, tc,_1,_2,_3,_4), mp);

    CPPUNIT_ASSERT( m0() );
    CPPUNIT_ASSERT( m1(1) );
    CPPUNIT_ASSERT( m2(1, 2.0) );
    CPPUNIT_ASSERT( m3(1, 2.0, false) );
    CPPUNIT_ASSERT( m4(1, 2.0, false,"hello") );
#endif
}

void MessageTest::testMessageFactory()
{
#if 0
    // Test the addition of 'simple' messages to the operation interface,
    // and retrieving it back in a new Message object.

    Message<double(void)> m0("m0", &MessageTask::m0, tc);
    Message<double(int)> m1("m1", &MessageTask::m1, tc);
    Message<double(int,double)> m2("m2", &MessageTask::m2, tc);

    TaskObject to("task");

    CPPUNIT_ASSERT( to.messages()->addMessage(&m0) );
    CPPUNIT_ASSERT( ! to.messages()->addMessage(&m0) );
    CPPUNIT_ASSERT( to.messages()->addMessage(&m1) );
    CPPUNIT_ASSERT( to.messages()->addMessage(&m2) );

    // test constructor
    Message<double(void)> mm0 = to.messages()->getMessage<double(void)>("m0");
    CPPUNIT_ASSERT( mm0.getMessageImpl() );
    CPPUNIT_ASSERT( mm0.ready() );

    // test operator=()
    Message<double(int)> mm1;
    mm1 = to.messages()->getMessage<double(int)>("m1");
    CPPUNIT_ASSERT( mm1.getMessageImpl() );
    CPPUNIT_ASSERT( mm1.ready() );

    Message<double(int,double)> mm2 = to.messages()->getMessage<double(int,double)>("m2");
    CPPUNIT_ASSERT( mm2.getMessageImpl() );
    CPPUNIT_ASSERT( mm2.ready() );

    // start the activity, such that messages are accepted.
    CPPUNIT_ASSERT( tsim->start()) ;
    // execute messages and check status:
    CPPUNIT_ASSERT_EQUAL( -1.0, mm0() );

    CPPUNIT_ASSERT_EQUAL( -2.0, mm1(1) );
    CPPUNIT_ASSERT_EQUAL( -3.0, mm2(1, 2.0) );

    // test error cases:
    // Add uninitialised message:
    Message<void(void)> mvoid;
    CPPUNIT_ASSERT(to.messages()->addMessage( &mvoid ) == false);
    mvoid = Message<void(void)>("voidm");
    CPPUNIT_ASSERT(to.messages()->addMessage( &mvoid ) == false);

    // wrong type 1:
    mvoid = to.messages()->getMessage<void(void)>("m1");
    CPPUNIT_ASSERT( mvoid.ready() == false );
    // wrong type 2:
    mvoid = to.messages()->getMessage<void(bool)>("m1");
    // wrong type 3:
    mvoid = to.messages()->getMessage<double(void)>("m0");
    CPPUNIT_ASSERT( mvoid.ready() == false );
    // non existing
    mvoid = to.messages()->getMessage<void(void)>("voidm");
    CPPUNIT_ASSERT( mvoid.ready() == false );

    // this line may not crash:
    mvoid();
#else
    CPPUNIT_ASSERT( false && "MessageFactory not implemented");
#endif

}

void MessageTest::testRemoteMessage()
{
#if 0
    Message<double(void)> m0;
    boost::shared_ptr<ActionInterface> implementation( new detail::RemoteMessage<double(void)>(tc->getObject("messages")->messages(),"m0") );
    m0 = implementation;
    CPPUNIT_ASSERT( m0.ready() );

    Message<double(int)> m1;
    implementation.reset( new detail::RemoteMessage<double(int)>(tc->getObject("messages")->messages(),"m1") );
    m1 = implementation;
    CPPUNIT_ASSERT( m1.ready() );

    CPPUNIT_ASSERT_EQUAL( -2.0, m1(1) );
    CPPUNIT_ASSERT_EQUAL( -1.0, m0() );
#else
    CPPUNIT_ASSERT( false && "RemoteMessage not implemented");
#endif
}

void MessageTest::testMessageC()
{
#if 0
    MessageC mc;
    double r = 0.0;
    mc = tc->getObject("messages")->messages()->create("m0").ret( r );
    CPPUNIT_ASSERT( mc.execute() );
    CPPUNIT_ASSERT( r == -1.0 );

    mc = tc->getObject("messages")->messages()->create("m2").argC(1).argC(1.0).ret( r );
    CPPUNIT_ASSERT( mc.execute() );
    CPPUNIT_ASSERT( r == -3.0 );

    mc = tc->getObject("messages")->messages()->create("m3").ret( r ).argC(1).argC(1.0).argC(true);
    CPPUNIT_ASSERT( mc.execute() );
    CPPUNIT_ASSERT( r == -4.0 );

#else
    CPPUNIT_ASSERT( false && "MessageC not implemented");
#endif
}


