#include "AnimTrack.h"
#include "AnimCurve.h"

#include "cinder/CinderAssert.h"

using namespace ci;
using namespace model;

template< typename T >
AnimCurve<T>::AnimCurve( const std::shared_ptr<AnimTrack>& parentTrack )
: mParentTrack( parentTrack.get() )
, mStartTime( std::numeric_limits<float>::max() )
, mEndTime( -std::numeric_limits<float>::max() )
, mAverageFrameDuration( 0.0f )
{
	mVirtualDuration = mParentTrack->getAnimDuration();
}

template< typename T >
void AnimCurve<T>::updateAverageFrameDuration( float time )
{
	//cumulative average
	mAverageFrameDuration = ( time - mAverageFrameDuration ) / float( mKeyframes.size() ) ;
}

template< typename T >
void AnimCurve<T>::addKeyframe( float time, T value )
{
	float duration = mParentTrack->getAnimDuration();
	
	CI_ASSERT( time <= duration && duration == mVirtualDuration );
	
	mKeyframes[time] = value;
	
	if( time < mStartTime ) {
		mStartTime = time;
	}
	if( time > mEndTime ) {
		mEndTime = time;
	}
	
	updateAverageFrameDuration( time );
	
	// Use an extra 'virtual keyframe' when the last frame is not equal to the first
	if( time == duration && value != mKeyframes.begin()->second
	   && mKeyframes.begin()->first == 0.0f ) {
		mKeyframes[time + mAverageFrameDuration] = mKeyframes.begin()->second;
		mVirtualDuration = time + mAverageFrameDuration;
	}
}

template< typename T >
T AnimCurve<T>::getValue(float time) const
{
	CI_ASSERT( !mKeyframes.empty() );
	if( mKeyframes.size() == 1 ) {
		return mKeyframes.begin()->second;
	}
	
	float cyclicTime = getCyclicTime( time );
	
	auto itnext =  mKeyframes.upper_bound( cyclicTime );
	auto itprev = ( itnext == mKeyframes.begin() ) ? mKeyframes.end() : itnext;
	itprev--;
	
	// no interpolation needed, we are right on the 'prev' keyframe
	if( cyclicTime == 0.0f || itprev->first == cyclicTime ) {
		return itprev->second;
	}
	
	float normalizedTime;
	if( itnext == mKeyframes.begin() ) {
		normalizedTime = cyclicTime / itnext->first;
	} else {
		normalizedTime = (cyclicTime - itprev->first) / (itnext->first - itprev->first);
	}
	
	CI_ASSERT( 0.0f < normalizedTime && 1.0f >= normalizedTime);
	return curveLerp( itprev->second, itnext->second, normalizedTime );
}

template< typename T >
inline float AnimCurve<T>::getCyclicTime( float time ) const
{
	if ( mParentTrack->getAnimDuration() == 0.0f ) return 0.0f;
	float n = time * mParentTrack->getAnimTicksPerSecond();
    return n - mVirtualDuration * floor( n / mVirtualDuration );
}

template< typename T >
inline T AnimCurve<T>::curveLerp( const T& start, const T& end, float time )
{
	return glm::mix( start, end, time );
}

template<>
inline quat AnimCurve<quat>::curveLerp( const quat& start, const quat& end, float time )
{
	return glm::slerp( start, end, time );
}

template<>
inline dquat AnimCurve<dquat>::curveLerp( const dquat& start, const dquat& end, float time )
{
	return glm::slerp( start, end, static_cast<double>( time ) );
}

//explicit instantiations
template class model::AnimCurve<vec3>;
template class model::AnimCurve<dvec3>;
template class model::AnimCurve<dquat>;
template class model::AnimCurve<quat>;

