//
//  Node.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Node.h"

namespace model {
	
	Node::Node( const ci::Vec3f& position, const ci::Quatf& rotation, const ci::Vec3f& scale,
			    const std::string& name, NodeRef parent, int level )
	: mInitialRelativePosition( position )
	, mInitialRelativeRotation( rotation )
	, mInitialRelativeScale( scale )
	, mRelativePosition( position )
	, mRelativeRotation( rotation )
	, mRelativeScale( scale )
	, mName( name )
	, mParent( parent )
	, mLevel( level )
	, mBoneIndex( -1 )
	, mTime( 0.0f )
	, mIsAnimated( false )
	, mNeedsUpdate( true )
	{
		
	}
	
	NodeRef Node::clone() const
	{
		NodeRef clone = NodeRef( new Node(mRelativePosition,
										  mRelativeRotation,
										  mRelativeScale,
										  mName,
										  nullptr,
										  mLevel ) );
		if (mOffset)
			clone->setOffsetMatrix( *mOffset );
		clone->setBoneIndex( getBoneIndex() );
		clone->mBoneIndex = mBoneIndex;
		
		//TODO: Copy animation data.
		//	clone->mIsAnimated = mIsAnimated;
		clone->animate( mTime );
		return clone;
	}
	
	void Node::computeTransformation( const ci::Vec3f& t, const ci::Quatf& r, const ci::Vec3f& s,  ci::Matrix44f* transformation )
	{
		*transformation = ci::Matrix44f::createScale( s );
		*transformation *= r;
		transformation->setTranslate( t );
	}
	
	void Node::setParent( NodeRef parent )
	{
		mParent = parent;
		requestSubtreeUpdate();
	}
	
	ci::Matrix44f Node::getRelativeTransformation()
	{
		ci::Matrix44f transformation;
		computeTransformation( mRelativePosition, mRelativeRotation, mRelativeScale, &transformation );
		return transformation;
	}
	
	ci::Matrix44f Node::getAbsoluteTransformation()
	{
		if( mNeedsUpdate ) {
			update();
		}
		ci::Matrix44f transformation;
		computeTransformation( mAbsolutePosition, mAbsoluteRotation, mAbsoluteScale, &transformation );
		return transformation;
	}
	
	ci::Vec3f& Node::getAbsolutePosition()
	{
		if( mNeedsUpdate ) {
			update();
		}
		return mAbsolutePosition;
	}
	
	const ci::Vec3f& Node::getAbsolutePosition() const
	{
		if( mNeedsUpdate ) {
			update();
		}
		return mAbsolutePosition;
	}
	
	ci::Quatf& Node::getAbsoluteRotation()
	{
		if( mNeedsUpdate ) {
			update();
		}
		return mAbsoluteRotation;
	}
	
	const ci::Quatf& Node::getAbsoluteRotation() const
	{
		if( mNeedsUpdate ) {
			update();
		}
		return mAbsoluteRotation;
	}
	
	ci::Vec3f& Node::getAbsoluteScale()
	{
		if( mNeedsUpdate ) {
			update();
		}
		return mAbsoluteScale;
	}
	
	const ci::Vec3f& Node::getAbsoluteScale() const
	{
		if( mNeedsUpdate ) {
			update();
		}
		return mAbsoluteScale;
	}

	void Node::setAbsolutePosition( const ci::Vec3f& pos )
	{
		mAbsolutePosition = pos;
		std::shared_ptr<Node> parent( mParent.lock() );
		if ( parent ) {
			setRelativePosition( mAbsolutePosition - parent->getAbsolutePosition() );
		} else {
			setRelativePosition( mAbsolutePosition );
		}
	}
	
	void Node::setRelativePosition( const ci::Vec3f& pos )
	{
		mRelativePosition = pos;
		requestSubtreeUpdate();
	}
	
	void Node::setRelativeRotation( const ci::Quatf& rotation )
	{
		mRelativeRotation = rotation;
		requestSubtreeUpdate();
	}
	
	void Node::setRelativeScale( const ci::Vec3f& scale )
	{
		mRelativeScale = scale;
		requestSubtreeUpdate();
	}
	
	void Node::resetToInitial()
	{
		mRelativePosition = mInitialRelativePosition;
		mRelativeRotation = mInitialRelativeRotation;
		mRelativeScale = mInitialRelativeScale;
	}
	
	void Node::addChild( NodeRef node )
	{
		mChildren.push_back( node );
	}
	
	void Node::addAnimTrack( int trackId, float duration, float ticksPerSecond )
	{
		mAnimTracks[trackId] = AnimTrack::create( duration, ticksPerSecond );
	}
	
	void Node::addPositionKeyframe( int trackId, float time, const ci::Vec3f& translation )
	{
		mAnimTracks[trackId]->mTranslationCurve->addKeyframe( time, translation );
	}
	
	void Node::addRotationKeyframe( int trackId, float time, const ci::Quatf& rotation )
	{
		mAnimTracks[trackId]->mRotationCurve->addKeyframe( time, rotation );
	}
	
	void Node::addScalingKeyframe( int trackId, float time, const ci::Vec3f& scaling )
	{
		mAnimTracks[trackId]->mScalingCurve->addKeyframe( time, scaling );
	}
	
	bool Node::hasAnimations( int trackId ) const
	{
		return mAnimTracks.count( trackId ) > 0;
	}
	
	void Node::update() const
	{
		// update orientation
		std::shared_ptr<Node> parent( mParent.lock() );
		if ( parent ) {
			const ci::Quatf& parentRotation = parent->getAbsoluteRotation();
			const ci::Vec3f& parentScale = parent->getAbsoluteScale();
			
			mAbsoluteRotation = mRelativeRotation * parentRotation;
			mAbsoluteScale = mRelativeScale * parentScale;
			
			// change position vector based on parent's rotation & scale
			mAbsolutePosition = ( parentScale * mRelativePosition ) * parentRotation;
			// add altered position vector to parent's
			mAbsolutePosition += parent->getAbsolutePosition();
		} else {
			mAbsoluteRotation = mRelativeRotation;
			mAbsoluteScale = mRelativeScale;
		}
		mNeedsUpdate = false;
	}

	
	
	void Node::animate( float time, int trackId )
	{
		mTime = time;
		mIsAnimated = false;
		if( hasAnimations( trackId ) ) {
			mAnimTracks[trackId]->getValues( mTime,
											 &mRelativePosition,
											 &mRelativeRotation,
											 &mRelativeScale);
			mIsAnimated = true;
		}
		requestSubtreeUpdate();
		update();
	}
	
	void Node::blendAnimate( float time, const std::unordered_map<int, float>& weights )
	{
		mTime = time;
		mIsAnimated = false;
		// We accumulate transformations from each animation track with a weighted sum
		// and use the result if at least on node was animated.
		ci::Vec3f weightedPosition;
		ci::Quatf weightedRotation;
		ci::Vec3f weightedScale;
		
		float w;
		for( auto kv : weights ) {
			if( hasAnimations( kv.first ) ) {
				mIsAnimated = true;
				w = kv.second;
				weightedPosition += w * mAnimTracks[ kv.first ]->getTranslation( time );
				weightedRotation = weightedRotation.slerp(w, mAnimTracks[ kv.first ]->getRotation( time ) );
				weightedScale	 += w * mAnimTracks[ kv.first ]->getScaling( time );
			}
		}
		if( mIsAnimated ) {
			mRelativePosition = weightedPosition;
			mRelativeRotation = weightedRotation;
			mRelativeScale = weightedScale;
		}
		requestSubtreeUpdate();
		update();
	}
	
	void Node::requestSubtreeUpdate()
	{
		mNeedsUpdate = true;
		
		for( auto childNode : mChildren ) {
			/* 
			 * We can prune out subsections of the tree which already need updates
			 * because a node that already needs updating will have traversed all its
			 * its children. Since all updates are usually done all at once, we don't
			 * retraverse the entire subtrees everytime.
			 */
			if( !childNode->mNeedsUpdate ) {
				childNode->requestSubtreeUpdate();
			}
		}
	}
	
} //end namespace model

