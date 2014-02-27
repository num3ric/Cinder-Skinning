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
	
Skeleton::Skeleton()
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
				  [&time, &weights] ( NodeRef n ) {
					  n->blendAnimate( time, weights );
				  } );
}

bool Skeleton::hasBone( const std::string& name ) const
{
	return mBoneNames.find( name ) != mBoneNames.end();
}

NodeRef Skeleton::getBone( const std::string& name ) const
{
	if( mBoneNames.count(name) > 0 ) {
		return mBoneNames.at( name );
	} else {
		return nullptr;
	}
}

NodeRef Skeleton::getNode(const std::string& name) const
{
	return findNode( name, mRootNode );
}

void Skeleton::addBone(const std::string &name, const NodeRef &bone)
{
	if( mBoneNames.count(name) == 0 ) {
		bone->setBoneIndex( mBoneNames.size() );
		mBoneNames[name] = bone;
	}
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
							   
							   auto parent = node->getParent().lock();
							   if( parent ) {
								   o << " parent:" << parent->getName();
							   }
							   o << std::endl;
							   o << "Position:" << node->getAbsolutePosition() << std::endl;
//							   o << "Transformation:" << std::endl << node->getInitialTransformation() << std::endl;
						   });
	return o;
}

} //end namespace model
