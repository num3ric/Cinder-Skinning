//
//  Skeleton.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Skeleton.h"

#include <assert.h>

Skeleton::Skeleton( NodeRef root, std::map<std::string, NodeRef> boneNames )
: mRootNode( root )
, mBoneNames( boneNames )
, mAnimationDuration( 0.0f )
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
//: mAnimationDuration( rhs.mAnimationDuration )
{
	mAnimationDuration = 0;
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

void Skeleton::update( float time )
{
	traverseNodes( mRootNode,
				  [ &time ] ( NodeRef n ) {
					  n->update( time );
				  } );
}

int Skeleton::getBoneIndex( const std::string& name ) const
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

bool Skeleton::isVisibleNode( const NodeRef& node ) const
{
	NodeRef parent = node->getParent();
	return  parent &&
			(hasBone( node->getName() ) || hasBone( parent->getName() ) ) &&
			parent->getInitialTransformation() != Matrix44f::identity();
}

void Skeleton::traverseNodes( const NodeRef& node, std::function<void(NodeRef)> visit ) const
{
	visit( node );	
	for( NodeRef child : node->getChildren() ) {
		traverseNodes(child, visit);
	}
}

void Skeleton::drawRelative(const NodeRef& node, const NodeRef& parent) const
{
	Matrix44f currentTransformation = node->getRelativeTransformation();
	
	gl::pushModelView();	
	if( isVisibleNode( node ) ) {
		gl::drawSkeletonNodeRelative( *node, Node::RenderMode::JOINTS );
	}
	gl::multModelView( currentTransformation );
	for( NodeRef child : node->getChildren() ) {
		drawRelative(child, node);
	}
	gl::popModelView();
}

void Skeleton::drawAbsolute( const NodeRef& node ) const
{
	traverseNodes( node,
				  [=] ( NodeRef n ) {
					  if( isVisibleNode( n ) ) {
						  gl::drawSkeletonNode( *n );
					  }
				  } );
}

void Skeleton::draw( bool relative, const std::string& name ) const
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	gl::disable( GL_LIGHTING );	
	NodeRef root = ( !name.empty() && hasBone( name ) ) ? mBoneNames.at(name) : mRootNode;
	if( relative ) {
		drawRelative( root );
	} else {
		drawAbsolute( root );
	}
	glPopClientAttrib();
	glPopAttrib();
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
							   o << "Transformation:" << std::endl << node->getInitialTransformation() << std::endl;
						   });
	return o;
}
