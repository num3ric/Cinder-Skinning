
//! maintest.cpp

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(SimpleTestInMainTestingModule)
{
	BOOST_CHECK(1 == 2);
}
