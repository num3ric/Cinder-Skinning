//
//  Node.h
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#pragma once

#include "AnimCurve.h"

#include "cinder/Matrix.h"
#include "cinder/gl/gl.h"
#include "cinder/Color.h"
#include "cinder/Vector.h"

#include <string>
#include <vector>

using namespace ci;

typedef std::shared_ptr<class Node> NodeRef;

class Node {
public:
	enum RenderMode { CONNECTED, JOINTS };
	
	explicit Node( Matrix44f absoluteTransformation, Matrix44f relativeTransformation, std::string name, NodeRef parent = nullptr, int level = 0 );
	
	NodeRef clone() const;
	
	void addChild( const NodeRef& Node );
	
	const NodeRef&		getParent() const { return mParent; }
	NodeRef&			getParent() { return mParent; }
	void				setParent( const NodeRef& parent ) { mParent = parent; }
	
	int		numChildren() const { return mChildren.size(); }
	bool	hasChildren() const { return !mChildren.empty(); }
	bool	hasParent()   const { return mParent != nullptr; }
	
	const std::vector<NodeRef>& getChildren() const { return mChildren; }
	std::vector<NodeRef>&		getChildren() { return mChildren; }
	
	const std::string&	getName() const { return mName; }
	void				setName( const std::string& name ) { mName = name; }
		
	const Matrix44f&	getRelativeTransformation() const { return mRelativeTransformation; }
	void				setRelativeTransformation( const Matrix44f& T ) { mRelativeTransformation = T; }
	
	const Matrix44f&	getAbsoluteTransformation() const { return mAbsoluteTransformation; }
	void				setAbsoluteTransformation( const Matrix44f& T ) { mRelativeTransformation = T;
																		  mAbsolutePosition = mAbsoluteTransformation * Vec3f::zero(); }
	
	const Vec3f&		getAbsolutePosition() const { return mAbsolutePosition; }
	
	const Matrix44f&	getInitialTransformation() const { return mInitialRelativeTransformation; }
	const Matrix44f&	getInitialAbsoluteTransformation() const { return mInitialAbsoluteTransformation; }
	const Vec3f&		getInitialAbsolutePosition() const { return mInitialAbsolutePosition; }
	
	int		getLevel() const { return mLevel; }
	void	setLevel( int level ) { mLevel = level; }
	int		getBoneIndex() const { return mBoneIndex; }
	void	setBoneIndex( int boneIndex ) { mBoneIndex = boneIndex; }
		
	void	addTranslationKeyframe( float time, const Vec3f& translation );
	void	addRotationKeyframe( float time, const Quatf& rotation );
	void	addScalingKeyframe( float time, const Vec3f& scaling );
	
	void	initAnimation( float duration, float ticksPerSecond );
	bool	isAnimated() const { return mIsAnimated; }
	float	getTime() { return mTime; }
	
	/*! 
	 *  Update the relative and absolute transformations using animation curves (if animated).
	 *	If the node is not animated, its absolute transformation is still updated based on its
	 *  parent because that in turn may be animated. No traversal is done.
	 */
	void	update( float time );
	
	/*!
	 *  Update the absolute transformation (and the convient absolute position) by concatenating
	 *  its relative transformation with with its parent's absolute transformation. No traversal
	 *  is done.
	 */
	void	updateAbsolute();
	
	void		setOffsetMatrix( const Matrix44f& offset ) { mOffset =  std::unique_ptr<Matrix44f>( new Matrix44f(offset) ); }
	const std::unique_ptr<Matrix44f>& getOffset() { return mOffset; }
	
	bool operator==( Node &rhs )
	{
		return (getName() == rhs.getName()
				&& getLevel() == rhs.getLevel() );
	}
	
	bool operator!=( Node &rhs )
	{
		return !(*this == rhs);
	}
protected:
	Vec3f		getAnimTranslation( float time ) const;
	Quatf		getAnimRotation( float time ) const;
	Vec3f		getAnimScaling( float time ) const;
	
	Matrix44f	mRelativeTransformation;
	Matrix44f	mAbsoluteTransformation;
	Vec3f		mAbsolutePosition;
	float		mTime;
	
	Matrix44f	mInitialRelativeTransformation;
	Matrix44f	mInitialAbsoluteTransformation;
	Vec3f		mInitialAbsolutePosition;
	
	std::unique_ptr<Matrix44f> mOffset;
	
	NodeRef		mParent;
	std::vector<NodeRef> mChildren;
	std::string	mName;
	int			mLevel;
	int			mBoneIndex;
	
	bool				mIsAnimated;
	AnimCurve<Vec3f>	mTranslationCurve;
	AnimCurve<Quatf>	mRotationCurve;
	AnimCurve<Vec3f>	mScalingCurve;
private:
	Node( const Node &rhs ); // private to prevent copying; use clone() method instead
	Node& operator=( const Node &rhs ); // not defined to prevent copying
};

namespace cinder {
	namespace gl {
		void drawCone( const Vec3f& start, const Vec3f& end );
		void drawSkeletonNode( const Node& node, Node::RenderMode mode = Node::RenderMode::CONNECTED );
		void drawSkeletonNodeRelative( const Node& node, Node::RenderMode mode = Node::RenderMode::CONNECTED );
	}
}
