//
//  Skeleton.h
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#pragma once

#include "Node.h"

#include <vector>
#include <map>
#include <functional>

namespace model {

typedef std::shared_ptr< class Skeleton> SkeletonRef;

/** 
 * The skeleton is composed of a hierachy of nodes, some of which are its bones.
 * Its bones are internally identified by a map from std::string names to NodeRef(s).
 *
 * The skeleton may be animated. If they are, the animation can be played using the
 * update method.
 **/
class Skeleton {
public:
	/** 
	  * FULL RenderMode draws every single node in the node hierarchy, even if it's not a bone.
	  * CLEANED applies several (seemingly arbitrary conditions for a node to be a visible part 
	  *         of the skeleton: such as being in the bone map, having bone parents whose initial
	  *         transformation are not the identity matrix.
	  * By default, FULL is applied.
	  */
	enum RenderMode { FULL, CLEANED };
	static enum RenderMode mRenderMode;

	static SkeletonRef create() { return SkeletonRef( new Skeleton() ); }
	static SkeletonRef create( NodeRef root, std::map<std::string, NodeRef> boneNames ) { return SkeletonRef( new Skeleton( root, boneNames ) ); }
	
	//! Deep copy of the node hierarchy and names map. Heavy, non-recommended operation. Prefer extracting different informations out of the same skeleton.
	virtual SkeletonRef clone() const;
	//! Update animation to time. (FIXME: If time exceeds bounds, only cyclic behavior for now.)
	void update( float time );
	
	//TODO: get pose @ time const-function.
	
	//! Render the skeleton.
	void draw( bool relative = false, const std::string& name = "" ) const;
	
	const NodeRef&	getRootNode() const { return mRootNode; }
	void			setRootNode( const NodeRef& root ) { mRootNode = root; }
	
	int				getBoneIndex( const std::string& name ) const;
	bool			hasBone( const std::string& name ) const;
	const NodeRef&	getBone( const std::string& name ) const;
	int				getNumBones() { return mBoneNames.size(); }
	
	void			insertBone( const std::string& name, const NodeRef& bone );
	
	const std::map<std::string, NodeRef>&	getBoneNames() const { return mBoneNames; }
	std::map<std::string, NodeRef>&			getBoneNames() { return mBoneNames; }
	
	NodeRef			getNode( const std::string& name) const;
	
	void			traverseNodes( const NodeRef& node, std::function<void(NodeRef)> visit ) const;
	
	float			mAnimationDuration; //temporary before better animation design
protected:
	Skeleton() { };
	explicit Skeleton( NodeRef root, std::map<std::string, NodeRef> boneNames );
	//! Determines whether the node should be rendered as part of the skeleton.
	bool	isVisibleNode( const NodeRef& node ) const;
	//! Draw the visible nodes/bones of the skeleton by traversing recursively its node transformation hierarchy.
	void	drawRelative( const NodeRef& node, const NodeRef& parent = nullptr ) const;
	//! Draw the visible nodes/bones of the skeleton by using its absolute bone positions.
	void	drawAbsolute( const NodeRef& node ) const;
	
	//! Find the node by traversing the hierarchy
	NodeRef findNode( const std::string& name, const NodeRef& node ) const;
	
	friend std::ostream& operator<<( std::ostream& o, const Skeleton& skeleton );

private:
	Skeleton( const Skeleton &rhs ); // private to prevent copying; use clone() method instead
	Skeleton& operator=( const Skeleton &rhs ); // not defined to prevent copying
	
	NodeRef mRootNode;
	std::map<std::string, NodeRef> mBoneNames;
};

extern std::ostream& operator<<( std::ostream& lhs, const Skeleton& rhs );

} //end namespace model
