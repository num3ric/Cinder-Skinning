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

#include <string>
#include <vector>

namespace model {

typedef std::shared_ptr<class Node> NodeRef;

class Node {
public:
	enum RenderMode { CONNECTED, JOINTS };
	
	explicit Node( const ci::Matrix44f& absoluteTransformation, const ci::Matrix44f& relativeTransformation, std::string name, NodeRef parent = nullptr, int level = 0 );
	
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
		
	const ci::Matrix44f&	getRelativeTransformation() const { return mRelativeTransformation; }
	void				setRelativeTransformation( const ci::Matrix44f& T ) { mRelativeTransformation = T; }
	
	const ci::Matrix44f&	getAbsoluteTransformation() const { return mAbsoluteTransformation; }
	void				setAbsoluteTransformation( const ci::Matrix44f& T ) { mRelativeTransformation = T;
																		  mAbsolutePosition = mAbsoluteTransformation * ci::Vec3f::zero(); }
	
	const ci::Vec3f&		getAbsolutePosition() const { return mAbsolutePosition; }
	
	const ci::Matrix44f&	getInitialTransformation() const { return mInitialRelativeTransformation; }
	const ci::Matrix44f&	getInitialAbsoluteTransformation() const { return mInitialAbsoluteTransformation; }
	const ci::Vec3f&		getInitialAbsolutePosition() const { return mInitialAbsolutePosition; }
	
	int		getLevel() const { return mLevel; }
	void	setLevel( int level ) { mLevel = level; }
	int		getBoneIndex() const { return mBoneIndex; }
	void	setBoneIndex( int boneIndex ) { mBoneIndex = boneIndex; }
		
	void	addTranslationKeyframe( float time, const ci::Vec3f& translation );
	void	addRotationKeyframe( float time, const ci::Quatf& rotation );
	void	addScalingKeyframe( float time, const ci::Vec3f& scaling );
	
	void	initAnimation( float duration, float ticksPerSecond );
	bool	isAnimated() const { return mIsAnimated; }
	float	getTime() { return mTime; }
	
	/*! 
	 *  Update the relative and absolute transformations using animation curves (if animated).
	 *	If the node is not animated, its absolute transformation is still updated based on its
	 *  parent because that in turn may be animated. No traversal is done.
	 */
	void	update( float time );
	
	void		setOffsetMatrix( const ci::Matrix44f& offset ) { mOffset =  std::unique_ptr<ci::Matrix44f>( new ci::Matrix44f(offset) ); }
	const std::unique_ptr<ci::Matrix44f>& getOffset() { return mOffset; }
	
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
	/*!
	 *  Update the absolute transformation (and the convient absolute position) by concatenating
	 *  its relative transformation with with its parent's absolute transformation. No traversal
	 *  is done.
	 */
	void	updateAbsolute();
	
	ci::Vec3f		getAnimTranslation( float time ) const;
	ci::Quatf		getAnimRotation( float time ) const;
	ci::Vec3f		getAnimScaling( float time ) const;
	
	ci::Matrix44f	mRelativeTransformation;
	ci::Matrix44f	mAbsoluteTransformation;
	ci::Vec3f		mAbsolutePosition;
	float			mTime;
	
	ci::Matrix44f	mInitialRelativeTransformation;
	ci::Matrix44f	mInitialAbsoluteTransformation;
	ci::Vec3f		mInitialAbsolutePosition;
	
	std::unique_ptr<ci::Matrix44f> mOffset;
	
	NodeRef		mParent;
	std::vector<NodeRef> mChildren;
	std::string	mName;
	int			mLevel;
	int			mBoneIndex;
	
	bool				mIsAnimated;
	AnimCurve<ci::Vec3f>	mTranslationCurve;
	AnimCurve<ci::Quatf>	mRotationCurve;
	AnimCurve<ci::Vec3f>	mScalingCurve;
private:
	Node( const Node &rhs ); // private to prevent copying; use clone() method instead
	Node& operator=( const Node &rhs ); // not defined to prevent copying
};
	
} //end namespace model

namespace cinder {
	namespace gl {
		void drawCone( const Vec3f& start, const Vec3f& end );
		void drawSkeletonNode( const model::Node& node, model::Node::RenderMode mode = model::Node::RenderMode::CONNECTED );
		void drawSkeletonNodeRelative( const model::Node& node, model::Node::RenderMode mode = model::Node::RenderMode::CONNECTED );
	}
}
