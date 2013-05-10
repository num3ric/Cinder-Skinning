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
		mWeights.clear();
		mAnimTime.stop();
		mAnimTime = 0.0f;
		mCurrentAnimId = animId;
	}
	
	void Actor::setBlendedAnimId( const std::unordered_map<int, float>& weights )
	{
		mWeights = weights;
		mAnimTime.stop();
		mAnimTime = 0.0f;
		for( auto kv : mWeights ) {
			mCurrentAnimId =  kv.first;
			break;
		}
	}
	
	void Actor::privateUpdate()
	{
		if( mWeights.empty() ) {
			setPose( mAnimTime(), mCurrentAnimId );
		} else {
			setBlendedPose( mAnimTime(), mWeights );
		}
		
	}
	
	void  Actor::playAnim()
	{
		float d = mAnimInfoMap[ mCurrentAnimId ].mDuration;
		ci::app::timeline().apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateUpdate, this ) );
	}
	
	void  Actor::loopAnim()
	{
		float d = mAnimInfoMap[ mCurrentAnimId ].mDuration;
		ci::app::timeline().apply(&mAnimTime, d, d )
						   .updateFn( std::bind(&Actor::privateUpdate, this ) )
						   .loop();
	}
	
	void  Actor::stop()
	{
		mAnimTime.stop();
	}

} //end namespace model
