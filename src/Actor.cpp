//
//  Actor.cpp
//  MultipleAnimationsDemo
//
//  Created by Éric Renaud-Houde on 2013-05-07.
//
//

#include "Actor.h"

namespace model {

	Actor::Actor()
	: mCurrentAnimId(0)
	, mAnimTime(0)
	{ }

	void Actor::setAnimInfo( int animId, const AnimInfo& animInfo )
	{
		mAnimInfoMap[animId] = animInfo;
	}
	
	void Actor::setAnimInfo( int animId, float duration, float ticksPerSecond, const std::string& name )
	{
		mAnimInfoMap[animId] = {duration, ticksPerSecond, name};
	}
	
	float Actor::getAnimDuration() const
	{
		return mAnimInfoMap.at( mCurrentAnimId ).mDuration;
	}
	
	const std::string& Actor::getAnimName() const
	{
		return mAnimInfoMap.at( mCurrentAnimId ).mName;
	}
	
	float Actor::getAnimTicksPerSecond() const
	{
		return mAnimInfoMap.at( mCurrentAnimId ).mTicksPerSecond;
	}
	
	void Actor::setAnimId( int animId )
	{
		mAnimTime.stop();
		mCurrentAnimId = animId;
	}
	
	void Actor::privateUpdate()
	{
		setPose( mAnimTime() );
	}
	
	void  Actor::playAnim()
	{
		float d = mAnimInfoMap[ mCurrentAnimId ].mDuration;
		mAnimTime = 0.0f;
		ci::app::timeline().apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateUpdate, this ) );
	}
	
	void  Actor::loopAnim()
	{
		float d = mAnimInfoMap[ mCurrentAnimId ].mDuration;
		mAnimTime = 0.0f;
		ci::app::timeline().apply(&mAnimTime, d, d )
						   .updateFn( std::bind(&Actor::privateUpdate, this ) )
						   .loop();
	}
	
	void  Actor::stop()
	{
		mAnimTime.stop();
	}

} //end namespace model
