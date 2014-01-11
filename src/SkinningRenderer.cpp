
#include "cinder/Color.h"
#include "cinder/gl/gl.h"
#include "cinder/Vector.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/Camera.h"
#include "cinder/app/AppNative.h"

#include "SkinningRenderer.h"
#include "Resources.h"
#include "SkinnedMesh.h"
#include "Skeleton.h"
#include "Node.h"
#include "SkinnedVboMesh.h"

namespace model {
	
	std::unique_ptr<SkinningRenderer> SkinningRenderer::mInstance = nullptr;
	
	std::once_flag SkinningRenderer::mOnceFlag;
		
	SkinningRenderer& SkinningRenderer::instance()
	{
		std::call_once(mOnceFlag,
					   [] {
						   mInstance.reset( new SkinningRenderer );
					   });
		return *mInstance.get();
	}
	
	SkinningRenderer::SkinningRenderer()
	{
		try {
			mSkinningShader = ci::gl::GlslProg::create( ci::app::loadResource(RES_SKINNING_VERT), ci::app::loadResource(RES_SKINNING_FRAG) );
		}
		catch( ci::gl::GlslProgCompileExc &exc ) {
			ci::app::console() << "Shader compile error: " << std::endl;
			ci::app::console() << exc.what();
		}
	}
	
	void SkinningRenderer::draw( std::shared_ptr<SkinnedMesh> skinnedMesh )
	{
		instance().privateDraw( skinnedMesh );
	}
	
	void SkinningRenderer::draw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh )
	{
		instance().privateDraw( skinnedVboMesh );
	}
	
	void SkinningRenderer::draw( std::shared_ptr<Skeleton> skeleton, bool absolute, const std::string& name )
	{
		instance().privateDraw( skeleton, absolute, name );
	}

	void SkinningRenderer::drawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera )
	{
		instance().privateDrawLabels( skeleton, camera );
	}
	
	void SkinningRenderer::drawSection( const AMeshSection& section, std::function<void()> drawMesh ) const
	{
		if( section.hasDefaultTransformation() ) {
			ci::gl::pushModelView();
			ci::gl::multModelView( section.getDefaultTranformation() );
		}
		
		if ( section.getMatInfo().mTexture ) {
			section.getMatInfo().mTexture.enableAndBind();
		}
		section.getMatInfo().mMaterial.apply();
		
		// Culling
		if (  section.getMatInfo().mTwoSided ) {
			ci::gl::disable( GL_CULL_FACE );
		} else {
			ci::gl::enable( GL_CULL_FACE );
		}
		
		drawMesh();
		
		// Texture Binding
		if (  section.getMatInfo().mTexture ) {
			section.getMatInfo().mTexture.unbind();
		}
		
		if( section.hasDefaultTransformation() ) {
			ci::gl::popModelView();
		}
	}
	
	void SkinningRenderer::privateDraw( std::shared_ptr<SkinnedMesh> skinnedMesh ) const
	{
		
		for( const SkinnedMesh::MeshSectionRef& section : skinnedMesh->getSections() ) {
			auto drawMesh = [section] {
				ci::gl::draw( section->mTriMesh );
			};
			drawSection( *section.get(), drawMesh);
		}
	}
	
	void SkinningRenderer::privateDraw(std::shared_ptr<SkinnedVboMesh> skinnedVboMesh ) const
	{
		
		for( const SkinnedVboMesh::MeshVboSectionRef& section : skinnedVboMesh->getSections() ) {
			auto drawMesh = [=] {
				mSkinningShader->bind();
				mSkinningShader->uniform( "isAnimated", section->isAnimated() );
				mSkinningShader->uniform( "texture", 0 );
				if( section->hasSkeleton() ) {
					mSkinningShader->uniform( "boneMatrices", section->mBoneMatricesPtr->data(), SkinnedVboMesh::MAXBONES );
					mSkinningShader->uniform( "invTransposeMatrices", section->mInvTransposeMatricesPtr->data(), SkinnedVboMesh::MAXBONES );
				}
				ci::gl::draw( section->getVboMesh() );
				//    ci::gl::drawRange(mVbo, 0, mVbo.getNumIndices()*3);
				mSkinningShader->unbind();
			};
			drawSection( *section.get(), drawMesh );
		}
	}
	
	void SkinningRenderer::drawRelative( SkeletonRef skeleton, const NodeRef& node, const NodeRef& parent) const
	{
		ci::Matrix44f currentTransformation = node->getRelativeTransformation();
		
		ci::gl::pushModelView();
		if( isVisibleNode( skeleton, node ) ) {
			drawSkeletonNodeRelative( node, Node::RenderMode::JOINTS );
		}
		ci::gl::multModelView( currentTransformation );
		for( NodeRef child : node->getChildren() ) {
			drawRelative(skeleton, child, node);
		}
		ci::gl::popModelView();
	}
	
	void SkinningRenderer::drawAbsolute( SkeletonRef skeleton, const NodeRef& node ) const
	{
		skeleton->traverseNodes( node,
					  [=] ( NodeRef n ) {
						  if( isVisibleNode( skeleton, n ) ) {
							  drawSkeletonNode( n );
						  }
					  } );
	}
	
	void SkinningRenderer::privateDraw( SkeletonRef skeleton, bool absolute, const std::string& name ) const
	{
		//TODO: remove push all_attribs for performance
		glPushAttrib( GL_ALL_ATTRIB_BITS );
		glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
		ci::gl::disable( GL_LIGHTING );
		NodeRef root = ( !name.empty() && skeleton->hasBone( name ) ) ? skeleton->getBoneNames().at(name) : skeleton->getRootNode();
		if( absolute ) {
			drawAbsolute( skeleton, root );
		} else {
			drawRelative( skeleton, root );
		}
		glPopClientAttrib();
		glPopAttrib();
	}
	
	void SkinningRenderer::privateDrawLabels( SkeletonRef skeleton, const ci::CameraPersp& camera ) const
	{
		ci::Matrix44f mv = ci::gl::getModelView();
		glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT );
		ci::gl::disable( GL_LIGHTING );
		ci::gl::enableAlphaBlending();
		ci::gl::disableDepthRead();
		ci::gl::disableDepthWrite();
		ci::gl::pushMatrices();
		ci::gl::setMatricesWindow( ci::app::getWindowSize() );
		skeleton->traverseNodes( skeleton->getRootNode(),
								[=] ( NodeRef n ) {
									if( isVisibleNode( skeleton, n ) ) {
										drawLabel(n, camera, mv );
									}
								} );
		ci::gl::popMatrices();
		glPopAttrib();
	}

	bool SkinningRenderer::isVisibleNode( SkeletonRef skeleton, const NodeRef& node ) const
	{
		if( Skeleton::mRenderMode == Skeleton::RenderMode::CLEANED) {
			NodeRef parent = node->getParent().lock();
			return  parent &&
			(skeleton->hasBone( node->getName() ) || skeleton->hasBone( parent->getName() ) ) &&
			parent->getInitialRelativePosition() != ci::Vec3f::zero();
		} else {
			return true;
		}
	}
	
	void SkinningRenderer::drawBone( const ci::Vec3f& start, const ci::Vec3f& end ) const
	{
		float dist = start.distance( end );
		ci::gl::drawSphere( start, 0.1f * dist , 4);
		ci::gl::color( ci::Color::white() );

		float maxGirth = 0.07f * dist;
		const int NUM_SEGMENTS = 4;
		ci::Vec3f boneVerts[NUM_SEGMENTS+2];
		glEnableClientState( GL_VERTEX_ARRAY );
		
		ci::Vec3f axis = ( start - end ).normalized();
		ci::Vec3f temp = ( axis.dot( ci::Vec3f::yAxis() ) > 0.999f ) ? axis.cross( ci::Vec3f::xAxis() ) : axis.cross( ci::Vec3f::yAxis() );
		ci::Vec3f left = 0.1f *  axis.cross( temp ).normalized();
		ci::Vec3f up = 0.1f * axis.cross( left ).normalized();
		
		glVertexPointer( 3, GL_FLOAT, 0, &boneVerts[0].x );
		boneVerts[0] = ci::Vec3f( end + axis * dist );
		boneVerts[1] = ci::Vec3f( end + axis * maxGirth + left );
		boneVerts[2] = ci::Vec3f( end + axis * maxGirth + up );
		boneVerts[3] = ci::Vec3f( end + axis * maxGirth - left );
		boneVerts[4] = ci::Vec3f( end + axis * maxGirth - up );
		boneVerts[5] = ci::Vec3f( end + axis * maxGirth + left );
		glDrawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
		
		glVertexPointer( 3, GL_FLOAT, 0, &boneVerts[0].x );
		boneVerts[0] = end;
		std::swap( boneVerts[2], boneVerts[4] );
		glDrawArrays( GL_TRIANGLE_FAN, 0, NUM_SEGMENTS+2 );
		
		glDisableClientState( GL_VERTEX_ARRAY );
	}
	
	void SkinningRenderer::drawJoint( const ci::Vec3f& nodePos ) const
	{
		float size = 0.2f;
		ci::gl::drawCube( nodePos, ci::Vec3f(size, size, size));
		size *= 5.0;
		ci::gl::color( ci::Color::white() );
		ci::gl::drawLine( nodePos, nodePos + ci::Vec3f(0, size, 0) );
	}
	
	void SkinningRenderer::drawSkeletonNode( const NodeRef& node, Node::RenderMode mode ) const
	{
		if( !node->hasParent() ) return;
		ci::Vec3f currentPos = node->getAbsolutePosition();
		ci::gl::color( node->isAnimated() ? ci::Color(1.0f, 0.0f, 0.0f) : ci::Color(0.0f, 1.0f, 0.0f) );
		if( mode == Node::RenderMode::CONNECTED ) {
			std::shared_ptr<Node> parent( node->getParent().lock() );
			if( parent ) {
				drawBone( currentPos, parent->getAbsolutePosition() );
			}
		} else if (mode == Node::RenderMode::JOINTS ) {
			drawJoint( currentPos );
		}
	}
	
	void SkinningRenderer::drawSkeletonNodeRelative( const NodeRef& node, Node::RenderMode mode ) const
	{
		ci::Vec3f currentPos = node->getRelativePosition();
		ci::gl::color( node->isAnimated() ? ci::Color(1.0f, 0.0f, 0.0f) : ci::Color(0.0f, 1.0f, 0.0f) );
		if( mode == Node::RenderMode::CONNECTED ) {
			drawBone( currentPos, ci::Vec3f::zero());
		} else if (mode == Node::RenderMode::JOINTS ) {
			drawJoint( currentPos );
		}
	}
	
	void SkinningRenderer::drawLabel( const NodeRef& node, const ci::CameraPersp& camera, const ci::Matrix44f& mv ) const
	{
		ci::Vec3f eyeCoord = mv * node->getAbsolutePosition();
		ci::Vec3f ndc = camera.getProjectionMatrix().transformPoint( eyeCoord );
		
		ci::Vec2f pos( ( ndc.x + 1.0f ) / 2.0f *  ci::app::getWindowWidth(), ( 1.0f - ( ndc.y + 1.0f ) / 2.0f ) * ci::app::getWindowHeight() );
		ci::gl::drawString( node->getName(), pos );
	}
	
}