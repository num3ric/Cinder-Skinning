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
	static NodeRef create( const glm::vec3& position, const glm::quat& rotation, glm::vec3 scale = glm::vec3( 1 ),
		const std::string& name = "", NodeRef parent = nullptr, size_t level = 0 );

	NodeRef clone() const;
	
	void addChild( NodeRef Node );
	
	const std::weak_ptr<Node>&		getParent() const { return mParent; }
	std::weak_ptr<Node>&			getParent() { return mParent; }
	void				setParent( NodeRef parent );
	
	size_t	getNumChildren() const { return mChildren.size(); }
	bool	hasChildren() const { return !mChildren.empty(); }
	bool	hasParent()   const { return !mParent.expired(); }
	
	const std::vector<NodeRef>& getChildren() const { return mChildren; }
	std::vector<NodeRef>&		getChildren() { return mChildren; }
	
	const std::string&	getName() const { return mName; }
	void				setName( const std::string& name ) { mName = name; }
	
	const glm::vec3&		getRelativePosition() const { return mRelativePosition; }
	const glm::quat&		getRelativeRotation() const { return mRelativeRotation; }
	const glm::vec3&		getRelativeScale() const { return mRelativeScale; }
	const glm::vec3&		getInitialRelativePosition() const { return mInitialRelativePosition; }
	const glm::quat&		getInitialRelativeRotation() const { return mInitialRelativeRotation; }
	const glm::vec3&		getInitialRelativeScale() const { return mInitialRelativeScale; }
	
	glm::mat4			getRelativeTransformation() const;
	glm::mat4			getAbsoluteTransformation() const;
	
	const glm::vec3&		getAbsolutePosition() const;
	const glm::quat&		getAbsoluteRotation() const;
	const glm::vec3&		getAbsoluteScale() const;
	
	//! Setting absolute positions is a more expensive operation: we have to compute relative ones
	void					setAbsolutePosition( const glm::vec3& pos );
	void					setRelativePosition( const glm::vec3& pos );
	//! Setting absolute rotations is a more expensive operation: we have to compute relative ones
	void					setAbsoluteRotation( const glm::quat& rotation );
	void					setRelativeRotation( const glm::quat& rotation );
	void					setRelativeScale( const glm::vec3& scale );
	
	void					resetToInitial();
	
	size_t	getLevel() const { return mLevel; }
	void	setLevel( size_t level ) { mLevel = level; }
	size_t	getBoneIndex() const { return mBoneIndex; }
	void	setBoneIndex( size_t boneIndex ) { mBoneIndex = boneIndex; }
	
	// Animation functions
	void	addAnimTrack( int trackId, float duration, float ticksPerSecond );
	void	addPositionKeyframe( int trackId, float time, const glm::vec3& position );
	void	addRotationKeyframe( int trackId, float time, const glm::quat& rotation  );
	void	addScalingKeyframe( int trackId, float time, const glm::vec3& scaling );
	
	bool	isAnimated() const { return mIsAnimated; }
	float	getTime() { return mTime; }
	
	/*! 
	 *  Update the relative and absolute transformations using animation curves (if animated).
	 *	If the node is not animated, its absolute transformation is still updated based on its
	 *  parent because that in turn may be animated. No traversal is done.
	 */
	void	animate( float time, int trackId = 0 );
	
	void	blendAnimate( float time, const std::unordered_map<int, float>& weights );
	
	void		setOffsetMatrix( const glm::mat4& offset ) { mOffset =  std::unique_ptr<glm::mat4>( new glm::mat4(offset) ); }
	const std::unique_ptr<glm::mat4>& getOffset() const { return mOffset; }
	
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
	explicit Node( const glm::vec3& position, const glm::quat& rotation, glm::vec3 scale,
		const std::string& name, NodeRef parent, size_t level );

	static void computeTransformation( const glm::vec3& t, const glm::quat& r, const glm::vec3& s,  glm::mat4* transformation );

	void	update() const;
	void	requestSubtreeUpdate();
	bool	hasAnimations( int trackId = 0 ) const;
	
	mutable bool		mNeedsUpdate;
	glm::vec3			mRelativePosition, mRelativeScale;
	glm::quat			mRelativeRotation;
	mutable glm::vec3	mAbsolutePosition, mAbsoluteScale;
	mutable glm::quat	mAbsoluteRotation;
	float		mTime;
	/*! A boolean indicating whether a node is animated.
	 * We are using a dynamic boolean flag because given a certain combination
	 * of animation blends, different nodes may become animated during runtime. 
	 */
	mutable bool			mIsAnimated;
	
	mutable glm::vec3 mInitialRelativePosition;
	mutable glm::quat mInitialRelativeRotation;
	mutable glm::vec3 mInitialRelativeScale;
	
	std::unique_ptr<glm::mat4> mOffset;
	
	std::weak_ptr<Node> mParent;
	std::vector<NodeRef> mChildren;
	std::string	mName;
	size_t		mLevel;
	size_t		mBoneIndex;
	
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
