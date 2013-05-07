//
//  AnimChannel.h
//  MultipleAnimationsDemo
//
//  Created by Éric Renaud-Houde on 2013-05-07.
//
//

#pragma once

#include "AnimCurve.h"

#include "cinder/Matrix44.h"

/*
 * FIXME: Here, against most good design recommendations, I would have liked the
 * anim curves to be aware of their container (channel) so that they could
 * query it for common data. I can't forward declaring template classes...
 * PImpl maybe? Convoluted?
 */

namespace model {

template< typename T >
class AnimCycle {
public:
	AnimCycle( float duration, float ticksPerSecond )
	{
		setAnimDuration( duration );
		setAnimTicksPerSecond( ticksPerSecond );
	}
	
	float	getAnimDuration() { return mDuration; }
	void	setAnimDuration( float duration )
	{
		mDuration = duration;
		mTranslationCurve.setAnimDuration( duration );
		mRotationCurve.setAnimDuration( duration );
		mScalingCurve.setAnimDuration( duration );
	}
	
	float	getAnimTicksPerSecond() { return mTicksPerSecond; }
	void	setAnimTicksPerSecond( float ticksPerSecond )
	{
		mTicksPerSecond = ticksPerSecond;
		mTranslationCurve.setAnimTicksPerSecond( ticksPerSecond );
		mRotationCurve.setAnimTicksPerSecond( ticksPerSecond );
		mScalingCurve.setAnimTicksPerSecond( ticksPerSecond );
	}
	
	ci::Matrix44<T> getTransformation( float time ) const
	{
		ci::Matrix44<T> t = ci::Matrix44<T>::createScale( mScalingCurve.getValue( time ) );
		t *= mRotationCurve.getValue( time ).toMatrix44();
		t.setTranslate( mTranslationCurve.getValue( time ) );
		return t;
	}
	
	AnimCurve<ci::Vec3<T>>			mTranslationCurve;
	AnimCurve<ci::Quaternion<T>>	mRotationCurve;
	AnimCurve<ci::Vec3<T>>			mScalingCurve;
private:
	float mDuration, mTicksPerSecond;
};

typedef AnimCycle<float> AnimCyclef;
typedef AnimCycle<double> AnimCycled;
	
}
