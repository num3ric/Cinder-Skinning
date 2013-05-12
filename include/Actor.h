#pragma once
#include "cinder/Timeline.h"
#include "cinder/app/AppNative.h"

#include <unordered_map>

namespace model {

class Actor {
public:
	struct AnimInfo {
		float mDuration;
		float mTicksPerSecond;
		std::string mName;
	};
	
	Actor();
	void			setAnimInfo( int trackId, const AnimInfo& animInfo );
	void			setAnimInfo( int trackId, float duration, float ticksPerSecond, const std::string& name );
	
	float				getAnimDuration() const;
	const std::string&	getAnimName() const;
	float				getAnimTicksPerSecond() const;
	
	int				getAnimTrackId() const { return mCurrentTrackId; }
	void			setAnimTrackId( int trackId );
	void			setBlendedAnimTrackId( const std::unordered_map<int, float>& weights );
	
	virtual void	setPose( float time, int trackId = 0 ) = 0;
	virtual void	setBlendedPose( float time, const std::unordered_map<int, float>& trackWeights ) = 0;
	void			playAnim();
	void			loopAnim();
	void			stop();
	
protected:
	void			privateUpdate();
	
	ci::Anim<float>	mAnimTime;
	int				mCurrentTrackId;
	std::unordered_map<int, AnimInfo> mAnimInfoMap;
	
	
	std::unordered_map<int, float> mAnimTrackWeights;
};

} //end namespace model

