

#pragma once
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"

#include <limits>
#include <math.h>
#include <map>
#include <assert.h>

using namespace ci;

template< typename T >
class AnimCurve {
public:
	AnimCurve();
	void	addKeyframe(float time, T value);
	T		getValue(float time) const;
	bool	empty() { return mKeyframes.empty(); }
	
	float	getAnimDuration() { return mDuration; }
	void	setAnimDuration( float duration ) { mDuration = duration; mVirtualDuration = duration; }
	float	getAnimTicksPerSecond() { return mTicksPerSecond; }
	void	setAnimTicksPerSecond( float ticksPerSecond ) { mTicksPerSecond = ticksPerSecond; }
private:
	static inline T		lerp( const T& start, const T& end, float time );
	static inline bool	isfinite( const ci::Vec3f& vec );
	inline float		getCyclicTime( float time ) const;
	void				updateAverageFrameDuration( float time );
	
	std::map<float, T> mKeyframes;
	float mStartTime, mEndTime;
	float mDuration, mVirtualDuration, mAverageFrameDuration, mTicksPerSecond;
};

template< typename T >
AnimCurve<T>::AnimCurve()
: mStartTime( std::numeric_limits<float>::max() )
, mEndTime( -std::numeric_limits<float>::max() )
, mDuration( 0.0f )
, mVirtualDuration( 0.0f )
, mAverageFrameDuration( 0.0f )
, mTicksPerSecond( 25.0f )
{
	
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
	assert( time <= mDuration && mDuration == mVirtualDuration );
	
	mKeyframes[time] = value;
	
	if( time < mStartTime ) {
		mStartTime = time;
	}	
	if( time > mEndTime ) {
		mEndTime = time;
	}
	
	updateAverageFrameDuration( time );
	
	// Use an extra 'virtual keyframe' when the last frame is not equal to the first
	if( time == mDuration && value != mKeyframes.begin()->second
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
	if ( mDuration == 0.0f ) return 0.0f;
	float n = time * mTicksPerSecond;
    return n - mVirtualDuration * floor( n / mVirtualDuration );
}

template< typename T >
inline T AnimCurve<T>::lerp( const T& start, const T& end, float time )
{
		return start * ( 1 - time ) + end * time; 
}

template< typename T >
inline bool AnimCurve<T>::isfinite( const ci::Vec3f& vec )
{
	return std::isfinite( vec.x ) && std::isfinite( vec.y ) && std::isfinite( vec.z );
}

template<>
inline Quatf AnimCurve<Quatf>::lerp( const Quatf& start, const Quatf& end, float time )
{
	//TODO: find the cause of these numerical errors.
	Quatf rotation = start.slerp(time, end);
	if ( !isfinite( rotation.getAxis() ) ) {
		rotation = Quatf::identity();
	}
	return rotation;
}

template<>
inline Quatd AnimCurve<Quatd>::lerp( const Quatd& start, const Quatd& end, float time )
{
	Quatd rotation = start.slerp(time, end);
	if ( !isfinite( rotation.getAxis() ) ) {
		rotation = Quatd::identity();
	}
	return rotation;
}
