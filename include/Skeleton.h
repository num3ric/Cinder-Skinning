//
//  Skeleton.h
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#pragma once

#include "Node.h"
#include "Actor.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace model {

typedef std::shared_ptr< class Skeleton> SkeletonRef;

/** 
 * The skeleton is composed of a hierachy of nodes, some of which are its bones.
 * Its bones are internally identified by a map from std::string names to NodeRef(s).
 **/
class Skeleton : public Actor {
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
	
	//! Deep copy of the node hierarchy and names map. Heavy, non-recommended operation. Prefer extracting different informations out of the same skeleton.
	virtual SkeletonRef clone() const;
	
	virtual void setPoseDefault() override;
	//! Update animation pose to specific time. (FIXME: If time exceeds bounds, only cyclic behavior for now.)
	virtual void setPose( float time, int animId = 0 ) override;
	
	virtual void setBlendedPose( float time, const std::unordered_map<int, float>& weights )override;
	
	const NodeRef&	getRootNode() const { return mRootNode; }
	void			setRootNode( const NodeRef& root ) { mRootNode = root; }
	
	bool			hasBone( const std::string& name ) const;
	NodeRef			getBone( const std::string& name ) const;
	int				getNumBones() { return mBoneNames.size(); }
	
	//! Adds bone node to name -> NodeRef map.
	void			addBone( const std::string& name, const NodeRef& bone );
	
	const std::unordered_map<std::string, NodeRef>&		getBoneNames() const { return mBoneNames; }
	std::unordered_map<std::string, NodeRef>&			getBoneNames() { return mBoneNames; }
	
	NodeRef			getNode( const std::string& name) const;
	
	void			traverseNodes( const NodeRef& node, std::function<void(NodeRef)> visit ) const;
protected:
	Skeleton();
	
	//! Find the node by traversing the hierarchy
	NodeRef findNode( const std::string& name, const NodeRef& node ) const;
	
	friend std::ostream& operator<<( std::ostream& o, const Skeleton& skeleton );

private:
	Skeleton( const Skeleton &rhs ); // private to prevent copying; use clone() method instead
	Skeleton& operator=( const Skeleton &rhs ); // not defined to prevent copying
	
	NodeRef mRootNode;
	std::unordered_map<std::string, NodeRef> mBoneNames;
};

extern std::ostream& operator<<( std::ostream& lhs, const Skeleton& rhs );

} //end namespace model
