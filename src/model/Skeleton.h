//
//  Skeleton.h
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#pragma once

#include "Node.h"
#include "ModelIo.h"

#include "cinder/AxisAlignedBox.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

namespace model {

/** 
 * The skeleton is composed of a hierachy of nodes, some of which are its bones.
 * Its bones are internally identified by a map from std::string names to NodeRef(s).
 **/
class Skeleton {
public:
	virtual ~Skeleton() { }

	/** 
	  * FULL RenderMode draws every single node in the node hierarchy, even if it's not a bone.
	  * CLEANED applies several (seemingly arbitrary conditions for a node to be a visible part 
	  *         of the skeleton: such as being in the bone map, having bone parents whose initial
	  *         transformation are not the identity matrix.
	  * By default, FULL is applied.
	  * BONES_ONLY uses only the bones.
	  */
	enum RenderMode { FULL, CLEANED, BONES_ONLY };
	static enum RenderMode sRenderMode;
	
	static SkeletonRef create( const NodeRef& rootNode );
	static SkeletonRef create( const NodeRef& rootNode, std::unordered_map<std::string, NodeRef> bones );
	
	//! Deep copy of the node hierarchy and names map. Heavy, non-recommended operation. Prefer extracting different informations out of the same skeleton.
	virtual SkeletonRef clone() const;
	
	const NodeRef&	getRootNode() const { return mRootNode; }
	NodeRef			getNode( const std::string& name) const;
	bool			isNodeVisible( const NodeRef& node ) const;
	void			traverseNodes( std::function<void(const NodeRef&)> visit ) const;
	static void		traverseNodes( const NodeRef& node, std::function<void(const NodeRef&)> visit );
	
	bool			hasBone( const std::string& name ) const;
	NodeRef			getBone( const std::string& name ) const;
	size_t			getNumBones() { return mBones.size(); }
	const std::unordered_map<std::string, NodeRef>&	getBones() const { return mBones; }
	
	//! Resulting box depends on the current Skeleton::RenderMode.
	ci::AxisAlignedBox calcBoundingBox() const;
protected:
	Skeleton( const NodeRef& rootNode );
	Skeleton( const NodeRef& rootNode, std::unordered_map<std::string, NodeRef> bones );
	
	//! Find the node by traversing the hierarchy
	NodeRef findNode( const std::string& name, const NodeRef& node ) const;
	
	friend std::ostream& operator<<( std::ostream& o, const Skeleton& skeleton );

private:
	Skeleton( const Skeleton &rhs ); // private to prevent copying; use clone() method instead
	Skeleton& operator=( const Skeleton &rhs ); // not defined to prevent copying
	
	NodeRef mRootNode;
	std::unordered_map<std::string, NodeRef> mBones;
};

extern std::ostream& operator<<( std::ostream& lhs, const Skeleton& rhs );

} //end namespace model
