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
	void			setAnimInfo( int animId, const AnimInfo& animInfo );
	void			setAnimInfo( int animId, float duration, float ticksPerSecond, const std::string& name );
	
	float				getAnimDuration() const;
	const std::string&	getAnimName() const;
	float				getAnimTicksPerSecond() const;
	
	int				getAnimId() const { return mCurrentAnimId; }
	void			setAnimId( int animId );
	virtual void	setPose( float time ) = 0;
	void			playAnim();
	void			loopAnim();
	void			stop();
	
private:
	void			privateUpdate();
	
	ci::Anim<float>	mAnimTime;
	int				mCurrentAnimId;
	
	std::unordered_map<int, AnimInfo> mAnimInfoMap;
};

} //end namespace model

