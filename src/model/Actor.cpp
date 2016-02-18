//
//  Actor.cpp
//  MultipleAnimationsDemo
//
//  Created by Éric Renaud-Houde on 2013-05-07.
//
//

#include "cinder/app/App.h"

#include "Actor.h"
#include "Skeleton.h"

using namespace model;
	
Actor::Actor( const Source& source, SkeletonRef skeleton )
{
	if( skeleton ) {
		mSkeleton = skeleton;
	} else {
		mSkeleton = Skeleton::create( source.getSkeletonRoot(), source.getSkeletonBones() );
	}
	
	size_t animId = 0;
	for( auto& animInfo : source.getAnimInfos() ) {
		mAnimInfoMap[animId] = AnimInfo( animInfo.getDuration(), animInfo.getTicksPerSecond(), animInfo.getName() );
		++animId;
	}
}

float Actor::getAnimDuration( int trackId ) const
{
	return mAnimInfoMap.at( trackId ).getDuration();
}

const std::string& Actor::getAnimName( int trackId ) const
{
	return mAnimInfoMap.at( trackId ).getName();
}

float Actor::getAnimTicksPerSecond( int trackId ) const
{
	return mAnimInfoMap.at( trackId ).getTicksPerSecond();
}

void Actor::resetPose()
{
	mSkeleton->traverseNodes( [] ( const NodeRef& n ) {
		n->resetToInitial();
	} );
	update();
}
void Actor::setPose( float time, int trackId )
{
	mSkeleton->traverseNodes( [&time, &trackId] ( const NodeRef& n ) {
		n->animate( time, trackId );
	} );
	update();
}

void Actor::setBlendedPose( float time, const std::unordered_map<int, float>& trackWeights )
{
	mSkeleton->traverseNodes( [&time, &trackWeights] ( const NodeRef& n ) {
		n->blendAnimate( time, trackWeights );
	} );
	update();
}
	
void Actor::updateImpl( int trackId )
{
	setPose( mAnimTime(), trackId );
}

void Actor::blendUpdateImpl( const std::unordered_map<int, float>& trackWeights )
{
	setBlendedPose( mAnimTime.value(), trackWeights );
}

void  Actor::playAnim( ci::Timeline& timeline, int trackId )
{
	mAnimTime = 0.0f;
	float d = mAnimInfoMap.at( trackId ).getDuration();
	timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::updateImpl, this, trackId ) );
}

void  Actor::playAnim( ci::Timeline& timeline, const std::unordered_map<int, float>& trackWeights )
{
	mAnimTime = 0.0f;
	float d = 0.0f;
	for( const auto& kv : mAnimInfoMap ) {
		float duration = kv.second.getDuration();
		if( duration > d )
			d = duration;
	}
	timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::blendUpdateImpl, this, trackWeights ) );
}

void  Actor::loopAnim( ci::Timeline& timeline, int trackId )
{
	mAnimTime = 0.0f;
	float d = mAnimInfoMap.at( trackId ).getDuration();
	timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::updateImpl, this, trackId ) ).loop();
}

void  Actor::loopAnim( ci::Timeline& timeline, const std::unordered_map<int, float>& trackWeights )
{
	mAnimTime = 0.0f;
	float d = 1.0f;
	for( const auto& kv : mAnimInfoMap ) {
		if( kv.second.getDuration() > d )
			d = kv.second.getDuration();
	}
	timeline.apply(&mAnimTime, d, d ).updateFn( std::bind(&Actor::blendUpdateImpl, this, trackWeights ) ).loop();
}

void  Actor::stop()
{
	mAnimTime.stop();
}

