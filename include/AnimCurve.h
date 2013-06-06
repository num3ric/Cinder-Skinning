

#pragma once
#include "cinder/Matrix44.h"
#include "cinder/Quaternion.h"

#include <limits>
#include <math.h>
#include <map>
#include <assert.h>

namespace model {
	
class AnimTrack;

template< typename T >
class AnimCurve {
public:
	AnimCurve() { }
	AnimCurve( const std::shared_ptr<AnimTrack>& parentTrack );
	void	addKeyframe(float time, T value);
	T		getValue(float time) const;
	bool	empty() { return mKeyframes.empty(); }
	
private:
	static inline T		lerp( const T& start, const T& end, float time );
	static inline bool	isFinite( const ci::Vec3f& vec );
	inline float		getCyclicTime( float time ) const;
	void				updateAverageFrameDuration( float time );
	
	AnimTrack*			mParentTrack;
	std::map<float, T>	mKeyframes;
	float mStartTime, mEndTime;
	float mVirtualDuration, mAverageFrameDuration;
};
	
} //end namespace model