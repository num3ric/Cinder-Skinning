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

namespace model {

class AnimTrack {
public:
	static std::shared_ptr<AnimTrack> create(float duration, float ticksPerSecond ) {
		std::shared_ptr<AnimTrack> track( new AnimTrack( duration, ticksPerSecond ) );
		track->mTranslationCurve = std::unique_ptr<AnimCurve<ci::Vec3f>>( new AnimCurve<ci::Vec3f>( track ) );
		track->mRotationCurve = std::unique_ptr<AnimCurve<ci::Quatf>>( new AnimCurve<ci::Quatf>( track ) );
		track->mScalingCurve = std::unique_ptr<AnimCurve<ci::Vec3f>>( new AnimCurve<ci::Vec3f>( track ) );
		return track;
	}
	
	float	getAnimDuration() { return mDuration; }
	void	setAnimDuration( float duration ) { mDuration = duration; }
	float	getAnimTicksPerSecond() { return mTicksPerSecond; }
	void	setAnimTicksPerSecond( float ticksPerSecond ) { mTicksPerSecond = ticksPerSecond; }
	
	ci::Vec3f getTranslation( float time ) const
	{
		return mTranslationCurve->getValue( time );
	}
	
	ci::Quatf getRotation( float time ) const
	{
		return mRotationCurve->getValue( time );
	}
	
	ci::Vec3f getScaling( float time ) const
	{
		return mScalingCurve->getValue( time );
	}
	
	void getValues( float time, ci::Vec3f* translate, ci::Quatf* rotation,  ci::Vec3f* scale )
	{
		*translate = mTranslationCurve->getValue( time );
		*rotation = mRotationCurve->getValue( time );
		*scale = mScalingCurve->getValue( time );
	}
	
	ci::Matrix44f getTransformation( float time ) const
	{
		ci::Matrix44f t = ci::Matrix44f::createScale( mScalingCurve->getValue( time ) );
		t *= mRotationCurve->getValue( time ).toMatrix44();
		t.setTranslate( mTranslationCurve->getValue( time ) );
		return t;
	}
	
	std::unique_ptr<AnimCurve<ci::Vec3f>>	mTranslationCurve;
	std::unique_ptr<AnimCurve<ci::Quatf>>	mRotationCurve;
	std::unique_ptr<AnimCurve<ci::Vec3f>>	mScalingCurve;
private:
	AnimTrack( float duration, float ticksPerSecond )
	: mDuration( duration )
	, mTicksPerSecond( ticksPerSecond )
	{ }
	
	// TODO: Implement copy/assignment operators before rendering them public again.
	AnimTrack(const AnimTrack& that);
	AnimTrack& operator=(const AnimTrack&);
	
	float mDuration, mTicksPerSecond;
};
	
}
