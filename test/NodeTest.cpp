
/*
 * Requires rebuilding boost by including 'test' library as such: –with-libraries=filesystem,system,date_time,test.
 * See http://libcinder.org/docs/v0.8.5/_cinder_boost.html for more detailed instructions.
 */


#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include "Node.h"

using namespace ci;
using namespace std;

typedef std::chrono::high_resolution_clock Clock;


BOOST_AUTO_TEST_CASE(SimpleTestInMainTestingModule)
{
	BOOST_CHECK(4 == 2);
	
}


BOOST_AUTO_TEST_CASE(PerformanceTest)
{
	{
		cout << "Performance test." << endl;
		auto start = Clock::now();
		
		int test = 0;
		for( long i=0; i<10000000; ++i ) {
			test = test + (i % 23423 );
		}
		auto end = Clock::now();
		
		auto nsdur = chrono::duration_cast<chrono::nanoseconds>( end - start );
		auto msdur = chrono::duration_cast<chrono::milliseconds>( end - start );
		cout << nsdur.count() << " ns,  " << msdur.count() << "ms" << endl;
	}
}