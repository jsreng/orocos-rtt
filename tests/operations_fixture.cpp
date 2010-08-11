#define BOOST_FUSION_INVOKE_MAX_ARITY 8
#define BOOST_FUSION_UNFUSED_MAX_ARITY 7

#include "operations_fixture.hpp"
#include <iostream>
#include <TaskContext.hpp>
#include <Method.hpp>
#include <Operation.hpp>
#include <interface/ServiceProvider.hpp>

using namespace std;
using namespace boost;
using namespace RTT;
using namespace RTT::detail;

OperationsFixture::OperationsFixture()
{
    ret = 0.0;
    i = -1;
    tc = new TaskContext("root");
    this->createMethodFactories(tc);
    tc->provides()->addAttribute("ret", ret );
    caller = new TaskContext("caller");
    caller->start();
    tc->start();
}

OperationsFixture::~OperationsFixture()
{
    tc->stop();
    delete tc;
    delete caller;
}

void OperationsFixture::createMethodFactories(TaskContext* target)
{
    ServiceProvider::shared_ptr dat = ServiceProvider::Create("test");
    dat->addAttribute("i", i);
    dat->addOperation("assert", &OperationsFixture::assertBool, this).doc("assert").arg("b", "bd");
    dat->addOperation("assertEqual", &OperationsFixture::assertEqual, this);
    dat->addOperation("increase", &OperationsFixture::increase, this).doc("Return increasing i");
    dat->addOperation("increaseCmd", &OperationsFixture::increase, this, OwnThread).doc("Return increasing i");
    dat->addOperation("resetI", &OperationsFixture::resetI, this).doc("ResetI i");
    dat->addOperation("assertMsg", &OperationsFixture::assertMsg, this).doc("Assert message").arg("bool", "").arg("text", "text");
    dat->addOperation("isTrue", &OperationsFixture::isTrue, this).doc("Identity function").arg("bool", "");
    dat->addOperation("fail", &OperationsFixture::fail, this).doc("Fails by throwing.");
    dat->addOperation("good", &OperationsFixture::good, this).doc("Returns true.");
    dat->addOperation("print", &OperationsFixture::print, this).doc("Print message");
    dat->addOperation("printNumber", &OperationsFixture::printNumber, this).doc("Print message + number");
    target->provides()->addService( dat );

    ServiceProvider::shared_ptr to = ServiceProvider::Create("methods");
    // ClientThread
    to->addOperation("m0r", &OperationsFixture::m0r, this).doc("M0r");
    to->addOperation("m0cr", &OperationsFixture::m0cr, this).doc("M0cr");
    to->addOperation("m1r", &OperationsFixture::m1r, this).doc("M1r");
    to->addOperation("m1cr", &OperationsFixture::m1cr, this).doc("M1cr");

    to->addOperation("vm0", &OperationsFixture::vm0, this).doc("VoidM0");
    to->addOperation("m0", &OperationsFixture::m0, this).doc("M0");
    to->addOperation("m1", &OperationsFixture::m1, this).doc("M1").arg("a", "ad");
    to->addOperation("m2", &OperationsFixture::m2, this).doc("M2").arg("a", "ad").arg("a", "ad");
    to->addOperation("m3", &OperationsFixture::m3, this).doc("M3").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("m4", &OperationsFixture::m4, this).doc("M4").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("m5", &OperationsFixture::m5, this).doc("M5").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("m6", &OperationsFixture::m6, this).doc("M6").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("m7", &OperationsFixture::m7, this).doc("M7").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");

    // OwnThread
    to->addOperation("o0r", &OperationsFixture::m0r, this, OwnThread).doc("M0r");
    to->addOperation("o0cr", &OperationsFixture::m0cr, this, OwnThread).doc("M0cr");
    to->addOperation("o1r", &OperationsFixture::m1r, this, OwnThread).doc("M1r");
    to->addOperation("o1cr", &OperationsFixture::m1cr, this, OwnThread).doc("M1cr");

    to->addOperation("o0", &OperationsFixture::m0, this, OwnThread).doc("M0");
    to->addOperation("o1", &OperationsFixture::m1, this, OwnThread).doc("M1").arg("a", "ad");
    to->addOperation("o2", &OperationsFixture::m2, this, OwnThread).doc("M2").arg("a", "ad").arg("a", "ad");
    to->addOperation("o3", &OperationsFixture::m3, this, OwnThread).doc("M3").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("o4", &OperationsFixture::m4, this, OwnThread).doc("M4").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("o5", &OperationsFixture::m5, this, OwnThread).doc("M5").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("o6", &OperationsFixture::m6, this, OwnThread).doc("M6").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    to->addOperation("o7", &OperationsFixture::m7, this, OwnThread).doc("M7").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad").arg("a", "ad");
    target->provides()->addService( to );
}
