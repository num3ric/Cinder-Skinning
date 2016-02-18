

#pragma once
#include "cinder/Matrix.h"
#include "cinder/Quaternion.h"

#include <limits>
#include <map>

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
	static inline T		curveLerp( const T& start, const T& end, float time );
	static inline bool	isFinite( const glm::vec3& vec );
	inline float		getCyclicTime( float time ) const;
	void				updateAverageFrameDuration( float time );
	
	AnimTrack*			mParentTrack;
	std::map<float, T>	mKeyframes;
	float mStartTime, mEndTime;
	float mVirtualDuration, mAverageFrameDuration;
};
	
} //end namespace model