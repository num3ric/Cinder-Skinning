//
//  AnimChannel.h
//  MultipleAnimationsDemo
//
//  Created by Éric Renaud-Houde on 2013-05-07.
//
//

#pragma once

#include "AnimCurve.h"

#include "cinder/CinderMath.h"

namespace model {

class AnimTrack {
public:
	static std::shared_ptr<AnimTrack> create(float duration, float ticksPerSecond ) {
		std::shared_ptr<AnimTrack> track( new AnimTrack( duration, ticksPerSecond ) );
		track->mTranslationCurve = std::unique_ptr<AnimCurve<glm::vec3>>( new AnimCurve<glm::vec3>( track ) );
		track->mRotationCurve = std::unique_ptr<AnimCurve<glm::quat>>( new AnimCurve<glm::quat>( track ) );
		track->mScalingCurve = std::unique_ptr<AnimCurve<glm::vec3>>( new AnimCurve<glm::vec3>( track ) );
		return track;
	}
	
	float	getAnimDuration() { return mDuration; }
	void	setAnimDuration( float duration ) { mDuration = duration; }
	float	getAnimTicksPerSecond() { return mTicksPerSecond; }
	void	setAnimTicksPerSecond( float ticksPerSecond ) { mTicksPerSecond = ticksPerSecond; }
	
	glm::vec3 getTranslation( float time ) const
	{
		return mTranslationCurve->getValue( time );
	}
	
	glm::quat getRotation( float time ) const
	{
		return mRotationCurve->getValue( time );
	}
	
	glm::vec3 getScaling( float time ) const
	{
		return mScalingCurve->getValue( time );
	}
	
	void getValues( float time, glm::vec3* translate, glm::quat* rotation,  glm::vec3* scale )
	{
		*translate = mTranslationCurve->getValue( time );
		*rotation = mRotationCurve->getValue( time );
		*scale = mScalingCurve->getValue( time );
	}
	
	glm::mat4 getTransformation( float time ) const
	{
		glm::mat4 t = glm::scale( mScalingCurve->getValue( time ) );
		t *= glm::toMat4( mRotationCurve->getValue( time ) );
		t += glm::translate( mTranslationCurve->getValue( time ) );
		return t;
	}
	
	std::unique_ptr<AnimCurve<glm::vec3>>	mTranslationCurve;
	std::unique_ptr<AnimCurve<glm::quat>>	mRotationCurve;
	std::unique_ptr<AnimCurve<glm::vec3>>	mScalingCurve;
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
