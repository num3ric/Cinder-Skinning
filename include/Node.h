//
//  Node.h
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#pragma once

#include "AnimTrack.h"

#include "cinder/Matrix44.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace model {

typedef std::shared_ptr<class Node> NodeRef;

class Node {
public:
	enum RenderMode { CONNECTED, JOINTS };
	
	explicit Node( const ci::Vec3f& position, const ci::Quatf& rotation, const ci::Vec3f& scale,
				   const std::string& name, NodeRef parent = nullptr, int level = 0 );
	
	NodeRef clone() const;
	
	void addChild( NodeRef Node );
	
	const std::weak_ptr<Node>&		getParent() const { return mParent; }
	std::weak_ptr<Node>&			getParent() { return mParent; }
	void				setParent( NodeRef parent );
	
	int		getNumChildren() const { return mChildren.size(); }
	bool	hasChildren() const { return !mChildren.empty(); }
	bool	hasParent()   const { return !mParent.expired(); }
	
	const std::vector<NodeRef>& getChildren() const { return mChildren; }
	std::vector<NodeRef>&		getChildren() { return mChildren; }
	
	const std::string&	getName() const { return mName; }
	void				setName( const std::string& name ) { mName = name; }
	
	ci::Vec3f&				getRelativePosition() { return mRelativePosition; }
	const ci::Vec3f&		getRelativePosition() const { return mRelativePosition; }
	ci::Quatf&				getRelativeRotation() { return mRelativeRotation; }
	const ci::Quatf&		getRelativeRotation() const { return mRelativeRotation; }
	ci::Vec3f&				getRelativeScale() { return mRelativeScale; }
	const ci::Vec3f&		getRelativeScale() const { return mRelativeScale; }
	ci::Vec3f&				getInitialRelativePosition() { return mInitialRelativePosition; }
	const ci::Vec3f&		getInitialRelativePosition() const { return mInitialRelativePosition; }
	ci::Quatf&				getInitialRelativeRotation() { return mInitialRelativeRotation; }
	const ci::Quatf&		getInitialRelativeRotation() const { return mInitialRelativeRotation; }
	ci::Vec3f&				getInitialRelativeScale() { return mInitialRelativeScale; }
	const ci::Vec3f&		getInitialRelativeScale() const { return mInitialRelativeScale; }
	
	ci::Matrix44f			getRelativeTransformation();
	ci::Matrix44f			getAbsoluteTransformation();
	
	ci::Vec3f&				getAbsolutePosition();
	const ci::Vec3f&		getAbsolutePosition() const;
	ci::Quatf&				getAbsoluteRotation();
	const ci::Quatf&		getAbsoluteRotation() const;
	ci::Vec3f&				getAbsoluteScale();
	const ci::Vec3f&		getAbsoluteScale() const;
	
	// Setting absolute positions is a more expensive operation: we have to compute relative ones
	void					setAbsolutePosition( const ci::Vec3f& pos );

	void					setRelativePosition( const ci::Vec3f& pos );
	void					setRelativeRotation( const ci::Quatf& rotation );
	void					setRelativeScale( const ci::Vec3f& scale );
	
	void					resetToInitial();
	
	int		getLevel() const { return mLevel; }
	void	setLevel( int level ) { mLevel = level; }
	int		getBoneIndex() const { return mBoneIndex; }
	void	setBoneIndex( int boneIndex ) { mBoneIndex = boneIndex; }
	
	// Animation functions
	void	addAnimTrack( int trackId, float duration, float ticksPerSecond );
	void	addPositionKeyframe( int trackId, float time, const ci::Vec3f& position );
	void	addRotationKeyframe( int trackId, float time, const ci::Quatf& rotation  );
	void	addScalingKeyframe( int trackId, float time, const ci::Vec3f& scaling );
	
	bool	isAnimated() const { return mIsAnimated; }
	float	getTime() { return mTime; }
	
	/*! 
	 *  Update the relative and absolute transformations using animation curves (if animated).
	 *	If the node is not animated, its absolute transformation is still updated based on its
	 *  parent because that in turn may be animated. No traversal is done.
	 */
	void	animate( float time, int trackId = 0 );
	
	void	blendAnimate( float time, const std::unordered_map<int, float>& weights );
	
	void		setOffsetMatrix( const ci::Matrix44f& offset ) { mOffset =  std::unique_ptr<ci::Matrix44f>( new ci::Matrix44f(offset) ); }
	const std::unique_ptr<ci::Matrix44f>& getOffset() { return mOffset; }
	
	bool operator==( Node &rhs )
	{
		return (	getName() == rhs.getName()
				&& getLevel() == rhs.getLevel() );
	}
	
	bool operator!=( Node &rhs )
	{
		return !(*this == rhs);
	}
protected:
	
	static void computeTransformation( const ci::Vec3f& t, const ci::Quatf& r, const ci::Vec3f& s,  ci::Matrix44f* transformation );

	void	update() const;
	void	requestSubtreeUpdate();
	bool	hasAnimations( int trackId = 0 ) const;
	
	mutable bool		mNeedsUpdate;
	ci::Vec3f			mRelativePosition, mRelativeScale;
	ci::Quatf			mRelativeRotation;
	mutable ci::Vec3f	mAbsolutePosition, mAbsoluteScale;
	mutable ci::Quatf	mAbsoluteRotation;
	float		mTime;
	/*! A boolean indicating whether a node is animated.
	 * We are using a dynamic boolean flag because given a certain combination
	 * of animation blends, different nodes may become animated during runtime. 
	 */
	mutable bool			mIsAnimated;
	
	mutable ci::Vec3f mInitialRelativePosition;
	mutable ci::Quatf mInitialRelativeRotation;
	mutable ci::Vec3f mInitialRelativeScale;
	
	std::unique_ptr<ci::Matrix44f> mOffset;
	
	std::weak_ptr<Node> mParent;
	std::vector<NodeRef> mChildren;
	std::string	mName;
	int			mLevel;
	int			mBoneIndex;
	
	/*!
	 * An unordered_map storing the different animation tracks with
	 * their specific int trackId key.
	 * This data-structure was chosen because a bone may be
	 * animated in some animation tracks, and not in others.
	 * */
	std::unordered_map< int, std::shared_ptr<AnimTrack> >	mAnimTracks;
private:
	Node( const Node &rhs ); // private to prevent copying; use clone() method instead
	Node& operator=( const Node &rhs ); // not defined to prevent copying
};
	
} //end namespace model
