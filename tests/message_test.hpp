#ifndef MESSAGE_TEST_H
#define MESSAGE_TEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <TaskContext.hpp>
#include <MessageProcessor.hpp>
#include <string>

using namespace RTT;

class MessageTask;

class MessageTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( MessageTest );

    CPPUNIT_TEST( testMessage );
    CPPUNIT_TEST( testMessageFactory );
    CPPUNIT_TEST( testRemoteMessage );
    CPPUNIT_TEST( testMessageC );

    CPPUNIT_TEST_SUITE_END();

    MessageTask* tc;
    ActivityInterface* act;
    MessageProcessor* mp;
    TaskObject* createMessageFactory();

public:

    void setUp();
    void tearDown();

    void testMessage();
    void testRemoteMessage();
    void testMessageC();
    void testMessageFactory();
};

#endif
