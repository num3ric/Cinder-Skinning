//
//  Skeleton.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Skeleton.h"

#include <assert.h>

namespace model {

Skeleton::RenderMode Skeleton::mRenderMode = Skeleton::RenderMode::FULL;

SkeletonRef Skeleton::create( const std::unordered_set<std::string>& boneNames )
{
	SkeletonRef inst( new Skeleton() );
	for( const std::string& name : boneNames ) {
		inst->insertBone( name, nullptr );
	}
	return inst;
}

Skeleton::Skeleton( NodeRef root, std::map<std::string, NodeRef> boneNames )
: mRootNode( root )
, mBoneNames( boneNames )
{ }

void cloneTraversal( const NodeRef& origin, NodeRef& copy )
{
	for( const auto& originChild : origin->getChildren() ) {
		NodeRef copyChild = originChild->clone();
		copyChild->setParent( copy );
		copy->addChild( copyChild );
		cloneTraversal( originChild, copyChild );
	}
}

Skeleton::Skeleton( const Skeleton &rhs )
{
	mRootNode = rhs.getRootNode()->clone();
	cloneTraversal( rhs.getRootNode(), mRootNode );
	
	for( auto& entry : rhs.getBoneNames() ) {
		std::string name = entry.first;
		mBoneNames[name] = getNode( name );
	}
}

SkeletonRef Skeleton::clone() const
{
	return SkeletonRef( new Skeleton( *this ) );
}

void Skeleton::setPoseDefault()
{
	traverseNodes( mRootNode,
				  [] ( NodeRef n ) {
					  n->resetToInitial();
				  } );
	
}

void Skeleton::setPose( float time, int animId )
{
	traverseNodes( mRootNode,
				  [&time, &animId] ( NodeRef n ) {
					  n->animate( time, animId );
				  } );
}
	
void Skeleton::setBlendedPose( float time, const std::unordered_map<int, float>& weights )
{
	traverseNodes( mRootNode,
				  [=] ( NodeRef n ) {
					  n->blendAnimate( time, weights );
				  } );
}

int Skeleton::findBoneIndex( const std::string& name ) const
{
	auto it = mBoneNames.find( name );
	return std::distance( mBoneNames.begin(), it );
}

bool Skeleton::hasBone( const std::string& name ) const
{
	return mBoneNames.find( name ) != mBoneNames.end();
}

const NodeRef& Skeleton::getBone( const std::string& name ) const
{
	return mBoneNames.at( name );
}

NodeRef Skeleton::getNode(const std::string& name) const
{
	return findNode( name, mRootNode );
}

void Skeleton::insertBone(const std::string &name, const NodeRef &bone)
{
	mBoneNames[name] = bone;
}

NodeRef Skeleton::findNode( const std::string& name, const NodeRef& node ) const
{
	if( node->getName() == name ) {
		return node;
	}
	for( const NodeRef& child : node->getChildren() ) {
		NodeRef foundNode = findNode( name, child );
		if (foundNode != nullptr) {
			return foundNode;
		}
	}
	return nullptr;
}

void Skeleton::traverseNodes( const NodeRef& node, std::function<void(NodeRef)> visit ) const
{
	visit( node );
	for( NodeRef child : node->getChildren() ) {
		traverseNodes(child, visit);
	}
}


std::ostream& operator<<( std::ostream& o, const Skeleton& skeleton )
{	
	skeleton.traverseNodes( skeleton.mRootNode,
						   [& o] (NodeRef node)
						   {
							   o << "Node:" << node->getName() << " level:" << node->getLevel();
							   if( node->hasParent() ) {
								   o << " parent:" << node->getParent()->getName();
							   }
							   o << std::endl;
							   o << "Position:" << node->getAbsolutePosition() << std::endl;
//							   o << "Transformation:" << std::endl << node->getInitialTransformation() << std::endl;
						   });
	return o;
}

} //end namespace model
