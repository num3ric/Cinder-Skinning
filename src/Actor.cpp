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
	: mCurrentTrackId(0)
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
		return mAnimInfoMap.at( mCurrentTrackId ).mDuration;
	}
	
	const std::string& Actor::getAnimName() const
	{
		return mAnimInfoMap.at( mCurrentTrackId ).mName;
	}
	
	float Actor::getAnimTicksPerSecond() const
	{
		return mAnimInfoMap.at( mCurrentTrackId ).mTicksPerSecond;
	}
	
	void Actor::setAnimTrackId( int animId )
	{
		mAnimTrackWeights.clear();
		mAnimTime.stop();
		mAnimTime = 0.0f;
		mCurrentTrackId = animId;
	}
	
	void Actor::setBlendedAnimTrackId( const std::unordered_map<int, float>& trackWeights )
	{
		mAnimTrackWeights = trackWeights;
		mAnimTime.stop();
		mAnimTime = 0.0f;
		for( auto kv : mAnimTrackWeights ) {
			mCurrentTrackId =  kv.first;
			break;
		}
	}
	
	void Actor::privateUpdate()
	{
		if( mAnimTrackWeights.empty() ) {
			setPose( mAnimTime(), mCurrentTrackId );
		} else {
			setBlendedPose( mAnimTime(), mAnimTrackWeights );
		}
		
	}
	
	void  Actor::playAnim()
	{
		float d = mAnimInfoMap[ mCurrentTrackId ].mDuration;
		ci::app::timeline().apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateUpdate, this ) );
	}
	
	void  Actor::loopAnim()
	{
		float d = mAnimInfoMap[ mCurrentTrackId ].mDuration;
		ci::app::timeline().apply(&mAnimTime, d, d )
						   .updateFn( std::bind(&Actor::privateUpdate, this ) )
						   .loop();
	}
	
	void  Actor::stop()
	{
		mAnimTime.stop();
	}

} //end namespace model
