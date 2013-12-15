
/*
 * Requires rebuilding boost by including 'test' library as such: –with-libraries=filesystem,system,date_time,test.
 * See http://libcinder.org/docs/v0.8.5/_cinder_boost.html for more detailed instructions.
 */


#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>
#include "Node.h"
#include "Skeleton.h"
#include "cinder/Vector.h"
#include "cinder/Quaternion.h"
#include <vector>

using namespace ci;
using namespace std;
using namespace model;

typedef std::chrono::high_resolution_clock Clock;


BOOST_AUTO_TEST_CASE(SimpleTestInMainTestingModule)
{
	BOOST_CHECK( true );
	
}


BOOST_AUTO_TEST_CASE(PerformanceTest)
{
	//Make a very large linked list of nodes
	
	SkeletonRef skeleton = Skeleton::create();
//	skeleton->
	
	vector<NodeRef> nodes;
	int num = 10000;
	for( int i=0; i<num; ++i ) {
		if( i==0) {
			nodes.push_back( make_shared<Node>( Vec3f::zero(), Quatf(), Vec3f::one(), "root", nullptr, i ) );
		} else {
			NodeRef child = make_shared<Node>( Vec3f::zero(), Quatf(), Vec3f::one(), "", nodes.back(), i );
			nodes.back()->addChild( child );
			nodes.push_back( child );
		}
	}
	
	//Verify that all nodes are properly linked
	for( auto& node : nodes ) {
		if( node->getLevel() != num - 1 ) {
			BOOST_CHECK( node->getNumChildren() == 1 );
		}
		
		if( node->getName() != "root" ) {
			BOOST_CHECK( node->getParent().lock() != nullptr );
		}
	}
	for( int r=0; r<100; ++r ) {
		nodes.front()->setRelativeRotation( Quatf( Vec3f(0.0f,1.0f,0.0f), M_PI ) );
		{
			cout << "Animate & update 10000 nodes: ";
			auto start = Clock::now();
			for( auto& node : nodes ) {
				node->animate(0.0f);
			}
			auto end = Clock::now();
			
			auto nsdur = chrono::duration_cast<chrono::nanoseconds>( end - start );
			auto msdur = chrono::duration_cast<chrono::milliseconds>( end - start );
			cout << nsdur.count() << " ns,  " << msdur.count() << "ms" << endl;
		}
	}
}