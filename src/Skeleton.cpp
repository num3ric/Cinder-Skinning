//
//  Skeleton.cpp
//  AssimpApp
//
//  Created by Éric Renaud-Houde on 2013-02-22.
//
//

#include "Skeleton.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"

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

bool Skeleton::isVisibleNode( const NodeRef& node ) const
{
	if( mRenderMode == RenderMode::CLEANED) {
		NodeRef parent = node->getParent();
		return  parent &&
				(hasBone( node->getName() ) || hasBone( parent->getName() ) ) &&
				 parent->getInitialRelativePosition() != ci::Vec3f::zero();
	} else {
		return true;
	}
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
	ci::Matrix44f currentTransformation = node->getRelativeTransformation();
	
	ci::gl::pushModelView();
	if( isVisibleNode( node ) ) {
		ci::gl::drawSkeletonNodeRelative( *node, Node::RenderMode::JOINTS );
	}
	ci::gl::multModelView( currentTransformation );
	for( NodeRef child : node->getChildren() ) {
		drawRelative(child, node);
	}
	ci::gl::popModelView();
}

void Skeleton::drawAbsolute( const NodeRef& node ) const
{
	traverseNodes( node,
				  [=] ( NodeRef n ) {
					  if( isVisibleNode( n ) ) {
						  ci::gl::drawSkeletonNode( *n );
					  }
				  } );
}

void Skeleton::draw( bool relative, const std::string& name ) const
{
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	ci::gl::disable( GL_LIGHTING );	
	NodeRef root = ( !name.empty() && hasBone( name ) ) ? mBoneNames.at(name) : mRootNode;
	if( relative ) {
		drawRelative( root );
	} else {
		drawAbsolute( root );
	}
	glPopClientAttrib();
	glPopAttrib();
}

void Skeleton::drawLabels( const ci::CameraPersp& camera )
{
	ci::Matrix44f mv = ci::gl::getModelView();
	glPushAttrib( GL_ALL_ATTRIB_BITS );
	glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
	ci::gl::disable( GL_LIGHTING );
	ci::gl::enableAlphaBlending();
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::setMatricesWindow( ci::app::getWindowSize() );
	traverseNodes( mRootNode,
				  [=] ( NodeRef n ) {
					  if( isVisibleNode( n ) ) {
						  ci::gl::drawLabel(*n, camera, mv );
					  }
				  } );
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
//							   o << "Transformation:" << std::endl << node->getInitialTransformation() << std::endl;
						   });
	return o;
}

} //end namespace model
