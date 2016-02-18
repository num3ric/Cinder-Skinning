//
//  Skeleton.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Skeleton.h"

#include "cinder/CinderAssert.h"

using namespace model;

Skeleton::RenderMode Skeleton::sRenderMode = Skeleton::RenderMode::FULL;

SkeletonRef Skeleton::create( const NodeRef &rootNode )
{
	return SkeletonRef( new Skeleton{ rootNode } );
}

SkeletonRef Skeleton::create( const NodeRef& rootNode, std::unordered_map<std::string, NodeRef> bones )
{
	return SkeletonRef( new Skeleton{ rootNode, bones } );
}

Skeleton::Skeleton( const NodeRef& rootNode )
: mRootNode( rootNode )
{
	traverseNodes( [this] ( const NodeRef& node ) {
		CI_ASSERT( ! node->getName().empty() );
		mBones.emplace( node->getName(), node );
	} );
}

Skeleton::Skeleton( const NodeRef& rootNode, std::unordered_map<std::string, NodeRef> bones )
: mRootNode( rootNode )
, mBones( bones )
{
	
}

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
	
	for( auto& entry : rhs.getBones() ) {
		std::string name = entry.first;
		mBones[name] = getNode( name );
	}
}

SkeletonRef Skeleton::clone() const
{
	return SkeletonRef( new Skeleton( *this ) );
}

bool Skeleton::hasBone( const std::string& name ) const
{
	return mBones.count( name ) > 0;
}

NodeRef Skeleton::getBone( const std::string& name ) const
{
	if( mBones.count(name) > 0 ) {
		return mBones.at( name );
	}
	return nullptr;
}

NodeRef Skeleton::getNode(const std::string& name) const
{
	return findNode( name, mRootNode );
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

void Skeleton::traverseNodes( std::function<void(const NodeRef&)> visit ) const
{
	traverseNodes( mRootNode, visit );
}

void Skeleton::traverseNodes( const NodeRef& node, std::function<void(const NodeRef&)> visit )
{
	visit( node );
	for( const NodeRef& child : node->getChildren() ) {
		traverseNodes(child, visit);
	}
}
	
bool Skeleton::isNodeVisible( const NodeRef& node ) const
{
	switch( sRenderMode ) {
		case FULL: return true;
		case CLEANED: {
			NodeRef parent = node->getParent().lock();
			return  parent &&
			( hasBone( node->getName() ) || hasBone( parent->getName() ) ) &&
			parent->getInitialRelativePosition() != ci::vec3( 0 );
		}
		case BONES_ONLY: return hasBone( node->getName() );
		default: return false;
	}
}
	
ci::AxisAlignedBox Skeleton::calcBoundingBox() const
{
	ci::AxisAlignedBox bb;
	traverseNodes( [&bb, this] ( const NodeRef& node ) {
						if( isNodeVisible( node ) ) {
							bb.include( node->getAbsolutePosition() );
						}
					} );
	
	return bb;
}


std::ostream& operator<<( std::ostream& o, const Skeleton& skeleton )
{	
	skeleton.traverseNodes( [& o] (const NodeRef& node)
						   {
							   o << "Node:" << node->getName() << " level:" << node->getLevel();
							   
							   auto parent = node->getParent().lock();
							   if( parent ) {
								   o << " parent:" << parent->getName();
							   }
							   o << std::endl;
							   o << "Position:" << node->getAbsolutePosition() << std::endl;
						   });
	return o;
}

