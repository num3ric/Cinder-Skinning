//
//  Actor.cpp
//  MultipleAnimationsDemo
//
//  Created by Éric Renaud-Houde on 2013-05-07.
//
//

#include "cinder/app/AppNative.h"

#include "Actor.h"


namespace model {

	Actor::Actor()
	: mAnimTime(0)
	{ }

	void Actor::setAnimInfo( int animId, const AnimInfo& animInfo )
	{
		mAnimInfoMap[animId] = animInfo;
	}
	
	void Actor::setAnimInfo( int animId, float duration, float ticksPerSecond, const std::string& name )
	{
		mAnimInfoMap[animId] = AnimInfo( duration, ticksPerSecond, name );
	}
	
	float Actor::getAnimDuration( int trackId ) const
	{
		return mAnimInfoMap.at( trackId ).mDuration;
	}
	
	const std::string& Actor::getAnimName( int trackId ) const
	{
		return mAnimInfoMap.at( trackId ).mName;
	}
	
	float Actor::getAnimTicksPerSecond( int trackId ) const
	{
		return mAnimInfoMap.at( trackId ).mTicksPerSecond;
	}
		
	void Actor::privateUpdate( int trackId )
	{
		setPose( mAnimTime(), trackId );
	}
	
	void Actor::privateBlendUpdate( const std::unordered_map<int, float>& trackWeights )
	{
		setBlendedPose( mAnimTime(), trackWeights );
	}
	
	void  Actor::playAnim( ci::Timeline& timeline, int trackId )
	{
		mAnimTime = 0.0f;
		float d = mAnimInfoMap[ trackId ].mDuration;
		timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateUpdate, this, trackId ) );
	}
	
	void  Actor::playAnim( ci::Timeline& timeline, const std::unordered_map<int, float>& trackWeights )
	{
		mAnimTime = 0.0f;
		float d = 0.0f;
		for( const auto& kv : mAnimInfoMap ) {
			if( kv.second.mDuration > d )
				d = kv.second.mDuration;
		}
		timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateBlendUpdate, this, trackWeights ) );
	}
	
	void  Actor::loopAnim( ci::Timeline& timeline, int trackId )
	{
		mAnimTime = 0.0f;
		float d = mAnimInfoMap[ trackId ].mDuration;
		timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateUpdate, this, trackId ) ).loop();
	}
	
	void  Actor::loopAnim( ci::Timeline& timeline, const std::unordered_map<int, float>& trackWeights )
	{
		mAnimTime = 0.0f;
		float d = 1.0f;
		for( const auto& kv : mAnimInfoMap ) {
			if( kv.second.mDuration > d )
				d = kv.second.mDuration;
		}
		timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::privateBlendUpdate, this, trackWeights ) ).loop();
	}
	
	void  Actor::stop()
	{
		mAnimTime.stop();
	}

} //end namespace model
