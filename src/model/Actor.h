#pragma once
#include <unordered_map>

#include "cinder/Timeline.h"
#include "cinder/Tween.h"

#include "ModelIo.h"

namespace model {

class Actor {
public:
	Actor( const Source& source, SkeletonRef skeleton );
	virtual ~Actor() { }
	
	virtual void		update() = 0;
	
	void				resetPose();
	void				setPose( float time, int trackId = 0 );
	void				setBlendedPose( float time, const std::unordered_map<int, float>& trackWeights );
		
	bool				hasAnimations() const { return ! mAnimInfoMap.empty(); }
	float				getAnimDuration( int trackId = 0 ) const;
	const std::string&	getAnimName( int trackId = 0 ) const;
	float				getAnimTicksPerSecond( int trackId = 0 ) const;
	
	SkeletonRef&		getSkeleton() { return mSkeleton; }
	const SkeletonRef&	getSkeleton() const { return mSkeleton; }
	
	void			playAnim( ci::Timeline& timeline, int trackId = 0 );
	void			playAnim( ci::Timeline& timeline, const std::unordered_map<int, float>& trackWeights );
	void			loopAnim( ci::Timeline& timeline, int trackId = 0 );
	void			loopAnim( ci::Timeline& timeline, const std::unordered_map<int, float>& trackWeights );
	void			stop();
	
protected:
	void			updateImpl( int trackId );
	void			blendUpdateImpl( const std::unordered_map<int, float>& trackWeights );
	
	SkeletonRef							mSkeleton;
	
	ci::Anim<float>						mAnimTime;
	std::unordered_map<int, AnimInfo>	mAnimInfoMap;
};

} //end namespace model

