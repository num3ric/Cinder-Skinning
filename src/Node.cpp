//
//  Node.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Node.h"

#include "cinder/gl/gl.h"
#include "cinder/Vector.h"

Node::Node( Matrix44f absoluteTransformation, Matrix44f relativeTransformation, std::string name, NodeRef parent, int level )
: mInitialAbsoluteTransformation( absoluteTransformation )
, mInitialAbsolutePosition( absoluteTransformation * Vec3f::zero() )
, mInitialRelativeTransformation( relativeTransformation )
, mName( name )
, mParent( parent )
, mLevel( level )
, mBoneIndex( -1 )
, mIsAnimated( false )
, mTime( 0.0f )
{
	mAbsoluteTransformation = mInitialAbsoluteTransformation;
	mAbsolutePosition = mInitialAbsolutePosition;
	mRelativeTransformation = mInitialRelativeTransformation;
}

NodeRef Node::clone() const
{
	NodeRef clone = NodeRef( new Node(mInitialAbsoluteTransformation,
									  mInitialRelativeTransformation,
									  mName,
									  nullptr,
									  mLevel ) );
	if (mOffset)
		clone->setOffsetMatrix( *mOffset );
	clone->setBoneIndex( getBoneIndex() );
	clone->mBoneIndex = mBoneIndex;
	
	//TODO: Copy animation data.
//	clone->mIsAnimated = mIsAnimated;
	clone->update( mTime );
	return clone;
}

void Node::addChild( const NodeRef& node )
{
	mChildren.push_back( node );
}

void Node::initAnimation( float duration, float ticksPerSecond )
{
	mTranslationCurve.setAnimDuration( duration );
	mTranslationCurve.setAnimTicksPerSecond( ticksPerSecond );
	mRotationCurve.setAnimDuration( duration );
	mRotationCurve.setAnimTicksPerSecond( ticksPerSecond );
	mScalingCurve.setAnimDuration( duration );
	mScalingCurve.setAnimTicksPerSecond( ticksPerSecond );
	mIsAnimated = true;
}

void Node::addTranslationKeyframe(float time, const Vec3f& translation)
{
	mTranslationCurve.addKeyframe( time, translation );
}

void Node::addRotationKeyframe(float time, const Quatf& rotation)
{
	mRotationCurve.addKeyframe( time, rotation );
}

void Node::addScalingKeyframe(float time, const Vec3f& scaling)
{
	mScalingCurve.addKeyframe( time, scaling );
}

Vec3f Node::getAnimTranslation( float time ) const
{
	return mTranslationCurve.getValue( time );
}

Quatf Node::getAnimRotation( float time ) const
{
	return mRotationCurve.getValue( time );
}

Vec3f Node::getAnimScaling( float time ) const
{
	return mScalingCurve.getValue( time );
}


void Node::updateAbsolute()
{
	mAbsoluteTransformation = mRelativeTransformation;
	if( hasParent() ) {
		mAbsoluteTransformation = getParent()->getAbsoluteTransformation() * mAbsoluteTransformation;
	}
	mAbsolutePosition = mAbsoluteTransformation * Vec3f::zero();
}


void Node::update( float time )
{
	if( isAnimated() ) {
		mRelativeTransformation = Matrix44f::createScale( getAnimScaling( time ) );
		mRelativeTransformation *= getAnimRotation( time ).toMatrix44();
		mRelativeTransformation.setTranslate( getAnimTranslation( time ) );
		updateAbsolute();
	} else {
		updateAbsolute();
	}	
	mTime = time;
}

namespace cinder {
	namespace gl {
		
		void drawCone( const Vec3f& start, const Vec3f& end ) {
			float headLength = start.distance(end);
			float headRadius = headLength / 20 ;
			const int NUM_SEGMENTS = 24;
			Vec3f coneVerts[NUM_SEGMENTS+2];
			glEnableClientState( GL_VERTEX_ARRAY );
			
			// Draw the cone
			Vec3f axis = ( start - end ).normalized();
			Vec3f temp = ( axis.dot( Vec3f::yAxis() ) > 0.999f ) ? axis.cross( Vec3f::xAxis() ) : axis.cross( Vec3f::yAxis() );
			Vec3f left = axis.cross( temp ).normalized();
			Vec3f up = axis.cross( left ).normalized();
			
			glVertexPointer( 3, GL_FLOAT, 0, &coneVerts[0].x );
			coneVerts[0] = Vec3f( end + axis * headLength );
			for( int s = 0; s <= NUM_SEGMENTS; ++s ) {
				float t = s / (float)NUM_SEGMENTS;
				coneVerts[s+1] = Vec3f( end + left * headRadius * math<float>::cos( t * 2 * 3.14159f )
									   + up * headRadius * math<float>::sin( t * 2 * 3.14159f ) );
			}
			glDrawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
			
			// draw the cap
			glVertexPointer( 3, GL_FLOAT, 0, &coneVerts[0].x );
			coneVerts[0] = end;
			for( int s = 0; s <= NUM_SEGMENTS; ++s ) {
				float t = s / (float)NUM_SEGMENTS;
				coneVerts[s+1] = Vec3f( end - left * headRadius * math<float>::cos( t * 2 * 3.14159f )
									   + up * headRadius * math<float>::sin( t * 2 * 3.14159f ) );
			}
			glDrawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
			
			glDisableClientState( GL_VERTEX_ARRAY );
		}
		
		namespace {
			void drawConnected( const Vec3f& nodePos, const Vec3f& parentPos ) {
				gl::drawSphere( nodePos, 0.2f , 4); //TODO: make size relative
				gl::color( Color::white() );
				gl::drawCone( nodePos, parentPos );
			}
			
			void drawJoint( const Vec3f& nodePos ) {
				float size = 0.2f;
				gl::drawCube( nodePos, Vec3f(size, size, size));
				size *= 5.0;
				gl::color( Color::white() );
				gl::drawLine( nodePos, nodePos + Vec3f(0, size, 0) );
			}
		}
		
		void drawSkeletonNode( const Node& node, Node::RenderMode mode )
		{
			if( !node.hasParent() ) return;
			Vec3f currentPos = node.getAbsolutePosition();
			Vec3f parentPos = node.getParent()->getAbsolutePosition();
			gl::color( node.isAnimated() ? Color(1.0f, 0.0f, 0.0f) : Color(0.0f, 1.0f, 0.0f) );
			if( mode == Node::RenderMode::CONNECTED ) {
				drawConnected( currentPos, parentPos);
			} else if (mode == Node::RenderMode::JOINTS ) {
				drawJoint( currentPos );
			}
		}
		
		void drawSkeletonNodeRelative( const Node& node, Node::RenderMode mode )
		{
			Vec3f currentPos = node.getRelativeTransformation() * Vec3f::zero();
			Vec3f parentPos = Vec3f::zero();
			gl::color( node.isAnimated() ? Color(1.0f, 0.0f, 0.0f) : Color(0.0f, 1.0f, 0.0f) );
			if( mode == Node::RenderMode::CONNECTED ) {
				drawConnected( currentPos, parentPos);
			} else if (mode == Node::RenderMode::JOINTS ) {
				drawJoint( currentPos );
			}
		}
	}
}
