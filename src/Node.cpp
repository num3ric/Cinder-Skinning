//
//  Node.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "cinder/app/AppNative.h"

#include "Node.h"

#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Camera.h"

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
		mAnimTracks[trackId] = std::make_shared<AnimTrackf>( duration, ticksPerSecond );
	}
	
	void Node::addPositionKeyframe( int trackId, float time, const ci::Vec3f& translation )
	{
		mAnimTracks[trackId]->mTranslationCurve.addKeyframe( time, translation );
	}
	
	void Node::addRotationKeyframe( int trackId, float time, const ci::Quatf& rotation )
	{
		mAnimTracks[trackId]->mRotationCurve.addKeyframe( time, rotation );
	}
	
	void Node::addScalingKeyframe( int trackId, float time, const ci::Vec3f& scaling )
	{
		mAnimTracks[trackId]->mScalingCurve.addKeyframe( time, scaling );
	}
	
	bool Node::hasAnimations( int trackId ) const
	{
		try {
			mAnimTracks.at( trackId );
			return true;
		} catch ( const std::out_of_range& ) {
			return false;
		}
	}	
	
	void Node::update() const
	{
		// update orientation
		if ( hasParent() ) {
			const ci::Quatf& parentRotation = mParent->getAbsoluteRotation();
			const ci::Vec3f& parentScale = mParent->getAbsoluteScale();
			
			mAbsoluteRotation = mRelativeRotation * parentRotation;
			mAbsoluteScale = mRelativeScale * parentScale;
			
			// change position vector based on parent's rotation & scale
			mAbsolutePosition = ( parentScale * mRelativePosition ) * parentRotation;
			// add altered position vector to parent's
			mAbsolutePosition += mParent->getAbsolutePosition();
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
		
		update();
	}
	
	void Node::requestSubtreeUpdate()
	{
		mNeedsUpdate = true;
		
		for( auto childNode : mChildren ) {
//			/* 
//			 * We can prune out subsections of the tree which already need updates
//			 * because a node that already needs updating will have traversed all its
//			 * its children. Since all updates are usually done all at once, we don't
//			 * retraverse the entire subtrees everytime.
//			 */
//			if( !childNode->mNeedsUpdate ) {
				childNode->requestSubtreeUpdate();
//			}
		}
	}
	
} //end namespace model

namespace cinder {
	namespace gl {
		
		void drawBone( const Vec3f& start, const Vec3f& end, float dist ) {
			if( dist <= 0 ) {
				dist = start.distance( end );
			}
			float maxGirth = 0.07f * dist;
			const int NUM_SEGMENTS = 4;
			Vec3f boneVerts[NUM_SEGMENTS+2];
			glEnableClientState( GL_VERTEX_ARRAY );
			
			Vec3f axis = ( start - end ).normalized();
			Vec3f temp = ( axis.dot( Vec3f::yAxis() ) > 0.999f ) ? axis.cross( Vec3f::xAxis() ) : axis.cross( Vec3f::yAxis() );
			Vec3f left = 0.1f *  axis.cross( temp ).normalized();
			Vec3f up = 0.1f * axis.cross( left ).normalized();
			
			glVertexPointer( 3, GL_FLOAT, 0, &boneVerts[0].x );
			boneVerts[0] = ci::Vec3f( end + axis * dist );
			boneVerts[1] = ci::Vec3f( end + axis * maxGirth + left );
			boneVerts[2] = ci::Vec3f( end + axis * maxGirth + up );
			boneVerts[3] = ci::Vec3f( end + axis * maxGirth - left );
			boneVerts[4] = ci::Vec3f( end + axis * maxGirth - up );
			boneVerts[5] = ci::Vec3f( end + axis * maxGirth + left );
			glDrawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
			
			glVertexPointer( 3, GL_FLOAT, 0, &boneVerts[0].x );
			boneVerts[0] = end;
			std::swap( boneVerts[2], boneVerts[4] );
			glDrawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
			
			glDisableClientState( GL_VERTEX_ARRAY );
		}
		
		void drawConnected( const Vec3f& nodePos, const Vec3f& parentPos ) {
			float dist = nodePos.distance( parentPos );
			drawSphere( nodePos, 0.1f * dist , 4);
			color( Color::white() );
			drawBone( nodePos, parentPos, dist );
		}
		
		void drawJoint( const Vec3f& nodePos ) {
			float size = 0.2f;
			drawCube( nodePos, Vec3f(size, size, size));
			size *= 5.0;
			color( Color::white() );
			drawLine( nodePos, nodePos + Vec3f(0, size, 0) );
		}
		
		void drawSkeletonNode( const model::Node& node, model::Node::RenderMode mode )
		{
			if( !node.hasParent() ) return;
			Vec3f currentPos = node.getAbsolutePosition();
			Vec3f parentPos = node.getParent()->getAbsolutePosition();
			color( node.isAnimated() ? Color(1.0f, 0.0f, 0.0f) : Color(0.0f, 1.0f, 0.0f) );
			if( mode == model::Node::RenderMode::CONNECTED ) {
				drawConnected( currentPos, parentPos);
			} else if (mode == model::Node::RenderMode::JOINTS ) {
				drawJoint( currentPos );
			}
		}
		
		void drawSkeletonNodeRelative( const model::Node& node, model::Node::RenderMode mode )
		{
			Vec3f currentPos = node.getRelativePosition();
			Vec3f parentPos = ci::Vec3f::zero();
			color( node.isAnimated() ? Color(1.0f, 0.0f, 0.0f) : Color(0.0f, 1.0f, 0.0f) );
			if( mode == model::Node::RenderMode::CONNECTED ) {
				drawConnected( currentPos, parentPos);
			} else if (mode == model::Node::RenderMode::JOINTS ) {
				drawJoint( currentPos );
			}
		}
		
		void drawLabel( const model::Node& node, const CameraPersp& camera, const ci::Matrix44f& mv )
		{
			Vec3f eyeCoord = mv * node.getAbsolutePosition();
			Vec3f ndc = camera.getProjectionMatrix().transformPoint( eyeCoord );
			
			Vec2f pos( ( ndc.x + 1.0f ) / 2.0f * app::getWindowWidth(), ( 1.0f - ( ndc.y + 1.0f ) / 2.0f ) * app::getWindowHeight() );
			drawString( node.getName(), pos );
		}
	}
}

