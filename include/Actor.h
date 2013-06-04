#pragma once
#include <unordered_map>

#include "cinder/Timeline.h"

namespace model {

class Actor {
public:
	struct AnimInfo {
		AnimInfo() {}
		AnimInfo( float duration, float ticksPerSecond, const std::string &name )
		: mDuration( duration ), mTicksPerSecond( ticksPerSecond ), mName( name )
		{}
		
		float mDuration;
		float mTicksPerSecond;
		std::string mName;
	};
	
	Actor();
	void			setAnimInfo( int trackId, const AnimInfo& animInfo );
	void			setAnimInfo( int trackId, float duration, float ticksPerSecond, const std::string& name );
	
	float				getAnimDuration( int trackId = 0 ) const;
	const std::string&	getAnimName( int trackId = 0 ) const;
	float				getAnimTicksPerSecond( int trackId = 0 ) const;
	
	virtual void	setPoseDefault() = 0;
	virtual void	setPose( float time, int trackId = 0 ) = 0;
	virtual void	setBlendedPose( float time, const std::unordered_map<int, float>& trackWeights ) = 0;
	
	void			playAnim( int trackId = 0 );
	void			playAnim( const std::unordered_map<int, float>& trackWeights );
	void			loopAnim( int trackId = 0 );
	void			loopAnim( const std::unordered_map<int, float>& trackWeights );
	void			stop();
	
	void			setAnimSpeed( float factor );
	
protected:
	void			privateUpdate( int trackId );
	void			privateBlendUpdate( const std::unordered_map<int, float>& trackWeights );
	
	
	ci::Anim<float>	mAnimTime;
	std::unordered_map<int, AnimInfo> mAnimInfoMap;
};

} //end namespace model

