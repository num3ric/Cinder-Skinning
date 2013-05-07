#pragma once

class Actor {
public:
	Actor()
	: mCycleId(0)
	, mAnimationDuration( 0.0f )
	{ }
	
	int getCycleId() const { return mCycleId; }
	void setCycleId( int cycleId ) { mCycleId = cycleId; }
	
public:
	int		mCycleId;
	float	mAnimationDuration; //temporary before better animation design
};
