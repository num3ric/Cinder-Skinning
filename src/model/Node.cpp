//
//  Node.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Node.h"

using namespace model;
using namespace ci;

NodeRef Node::create( const vec3& position, const quat& rotation, vec3 scale,
					 const std::string& name, NodeRef parent, size_t level )
{
	return NodeRef( new Node( position, rotation, scale, name, parent, level ) );
}

Node::Node( const vec3& position, const quat& rotation, vec3 scale,
		   const std::string& name, NodeRef parent, size_t level )
: mInitialRelativePosition( position )
, mInitialRelativeRotation( rotation )
, mInitialRelativeScale( scale )
, mRelativePosition( position )
, mRelativeRotation( rotation )
, mRelativeScale( scale )
, mName( name )
, mParent( parent )
, mLevel( level )
, mBoneIndex( 0 )
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
	if( mOffset )
		clone->setOffsetMatrix( *mOffset );
	clone->setBoneIndex( getBoneIndex() );
	clone->mBoneIndex = mBoneIndex;
	
	//TODO: Copy animation data.
	//	clone->mIsAnimated = mIsAnimated;
	clone->animate( mTime );
	return clone;
}

void Node::computeTransformation( const vec3& t, const quat& r, const vec3& s,  mat4* transformation )
{
	*transformation = glm::translate( t ) * glm::toMat4( r ) * glm::scale( s );
}

void Node::setParent( NodeRef parent )
{
	mParent = parent;
	requestSubtreeUpdate();
}

mat4 Node::getRelativeTransformation() const
{
	mat4 transformation;
	computeTransformation( mRelativePosition, mRelativeRotation, mRelativeScale, &transformation );
	return transformation;
}

mat4 Node::getAbsoluteTransformation() const
{
	if( mNeedsUpdate ) {
		update();
	}
	mat4 transformation;
	computeTransformation( mAbsolutePosition, mAbsoluteRotation, mAbsoluteScale, &transformation );
	return transformation;
}

const vec3& Node::getAbsolutePosition() const
{
	if( mNeedsUpdate ) {
		update();
	}
	return mAbsolutePosition;
}

const quat& Node::getAbsoluteRotation() const
{
	if( mNeedsUpdate ) {
		update();
	}
	return mAbsoluteRotation;
}

const vec3& Node::getAbsoluteScale() const
{
	if( mNeedsUpdate ) {
		update();
	}
	return mAbsoluteScale;
}

void Node::setAbsolutePosition( const vec3& pos )
{
	mAbsolutePosition = pos;
	std::shared_ptr<Node> parent( mParent.lock() );
	if ( parent ) {
		setRelativePosition( mAbsolutePosition - parent->getAbsolutePosition() );
	} else {
		setRelativePosition( mAbsolutePosition );
	}
}

void Node::setRelativePosition( const vec3& pos )
{
	mRelativePosition = pos;
	requestSubtreeUpdate();
}

void Node::setAbsoluteRotation( const quat& rotation )
{
	mAbsoluteRotation = rotation;
	std::shared_ptr<Node> parent( mParent.lock() );
	if ( parent ) {
		setRelativeRotation( glm::inverse( parent->getAbsoluteRotation() ) * mAbsoluteRotation );
	} else {
		setRelativeRotation( mAbsoluteRotation );
	}
}

void Node::setRelativeRotation( const quat& rotation )
{
	mRelativeRotation = rotation;
	requestSubtreeUpdate();
}

void Node::setRelativeScale( const vec3& scale )
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

void Node::addPositionKeyframe( int trackId, float time, const vec3& translation )
{
	mAnimTracks.at( trackId )->mTranslationCurve->addKeyframe( time, translation );
}

void Node::addRotationKeyframe( int trackId, float time, const quat& rotation )
{
	mAnimTracks.at( trackId )->mRotationCurve->addKeyframe( time, rotation );
}

void Node::addScalingKeyframe( int trackId, float time, const vec3& scaling )
{
	mAnimTracks.at( trackId )->mScalingCurve->addKeyframe( time, scaling );
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
		const quat& parentRotation = parent->getAbsoluteRotation();
		const vec3& parentScale = parent->getAbsoluteScale();
		
		mAbsoluteRotation = parentRotation * mRelativeRotation;
		mAbsoluteScale = mRelativeScale * parentScale;
		
		// change position vector based on parent's rotation & scale
		mAbsolutePosition = parentRotation * ( parentScale * mRelativePosition );
		// add altered position vector to parent's
		mAbsolutePosition += parent->getAbsolutePosition();
	} else {
		mAbsolutePosition = mRelativePosition;
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
	vec3 weightedPosition;
	quat weightedRotation;
	vec3 weightedScale;
	
	float w;
	for( auto& kv : weights ) {
		if( hasAnimations( kv.first ) ) {
			mIsAnimated = true;
			w = kv.second;
			weightedPosition += w * mAnimTracks.at( kv.first )->getTranslation( time );
			weightedRotation = glm::slerp( weightedRotation, mAnimTracks.at( kv.first )->getRotation( time ), w );
			weightedScale	 += w * mAnimTracks.at( kv.first )->getScaling( time );
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

