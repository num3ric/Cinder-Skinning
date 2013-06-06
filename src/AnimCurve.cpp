
#include "AnimTrack.h"
#include "AnimCurve.h"

namespace model {

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
	
	assert( time <= duration && duration == mVirtualDuration );
	
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
	assert( !mKeyframes.empty() );
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
	
	assert( 0.0f < normalizedTime && 1.0f >= normalizedTime);
	return lerp( itprev->second, itnext->second, normalizedTime );
}

template< typename T >
inline float AnimCurve<T>::getCyclicTime( float time ) const
{
	if ( mParentTrack->getAnimDuration() == 0.0f ) return 0.0f;
	float n = time * mParentTrack->getAnimTicksPerSecond();
    return n - mVirtualDuration * floor( n / mVirtualDuration );
}

template< typename T >
inline T AnimCurve<T>::lerp( const T& start, const T& end, float time )
{
	return start * ( 1 - time ) + end * time;
}

template< typename T >
inline bool AnimCurve<T>::isFinite( const ci::Vec3f& vec )
{
	return std::isfinite( vec.x ) && std::isfinite( vec.y ) && std::isfinite( vec.z );
}

template<>
inline ci::Quatf AnimCurve<ci::Quatf>::lerp( const ci::Quatf& start, const ci::Quatf& end, float time )
{
	//TODO: find the cause of these numerical errors.
	ci::Quatf rotation = start.slerp(time, end);
	if ( !isFinite( rotation.getAxis() ) ) {
		rotation = ci::Quatf::identity();
	}
	return rotation;
}

template<>
inline ci::Quatd AnimCurve<ci::Quatd>::lerp( const ci::Quatd& start, const ci::Quatd& end, float time )
{
	ci::Quatd rotation = start.slerp(time, end);
	if ( !isFinite( rotation.getAxis() ) ) {
		rotation = ci::Quatd::identity();
	}
	return rotation;
}
	
//explicit instantiations
template class AnimCurve<ci::Vec3f>;
template class AnimCurve<ci::Vec3d>;
template class AnimCurve<ci::Quatd>;
template class AnimCurve<ci::Quatf>;

} //end namespace model