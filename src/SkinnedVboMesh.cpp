//
//  SkinnedVboMesh.cpp
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-05.
//
//

#include "SkinnedVboMesh.h"
#include "ModelTargetSkinnedVboMesh.h"

#include "Skeleton.h"
#include "SkinningRenderer.h"
#include "Resources.h"

namespace model {

SkinnedVboMesh::MeshSection::MeshSection()
: AMeshSection()
{ }

void SkinnedVboMesh::MeshSection::setVboMesh( size_t numVertices, size_t numIndices, ci::gl::VboMesh::Layout layout, GLenum primitiveType )
{
	mVboMesh = ci::gl::VboMesh( numVertices, numIndices, layout, primitiveType );
}

void SkinnedVboMesh::MeshSection::updateMesh( bool enableSkinning )
{
	if( enableSkinning ) {
		int i = 0;
		for( const auto& entry : mSkeleton->getBoneNames() ) {
			if( i >= MAXBONES )
				break;
			NodeRef bone = entry.second;
			(*mBoneMatricesPtr)[bone->getBoneIndex()] = bone->getAbsoluteTransformation() * *bone->getOffset();
			(*mInvTransposeMatricesPtr)[bone->getBoneIndex()] = (*mBoneMatricesPtr)[bone->getBoneIndex()].orthonormalInverted();
			(*mInvTransposeMatricesPtr)[bone->getBoneIndex()].transpose();
			++i;
		}
		mIsAnimated = true;
	} else if( mIsAnimated ) {
		mIsAnimated = false;
	}
}

SkinnedVboMeshRef SkinnedVboMesh::create( ModelSourceRef modelSource, SkeletonRef skeleton, ci::gl::GlslProgRef skinningShader )
{
	if( skinningShader ) {
		return SkinnedVboMeshRef( new SkinnedVboMesh( modelSource, skinningShader, skeleton ) );
	} else {
		return SkinnedVboMeshRef( new SkinnedVboMesh( modelSource, SkinningRenderer::instance().getShader(), skeleton ) );
	}
}

SkinnedVboMesh::SkinnedVboMesh( ModelSourceRef modelSource, ci::gl::GlslProgRef skinningShader, SkeletonRef skeleton )
: mEnableSkinning( true )
, mSkinningShader( skinningShader )
{
	assert( modelSource->getNumSections() > 0 );
	
	for( unsigned int i = 0; i< modelSource->getNumSections(); ++i ) {
		MeshVboSectionRef section = std::make_shared<SkinnedVboMesh::MeshSection>();
		ci::gl::VboMesh::Layout layout;
		layout.setStaticIndices();
		//positions
		layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT3, 0 ));
		
		if( modelSource->hasNormals(i) ) {
			//normals
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT3, 0 ));
		}
		
		if( modelSource->hasMaterials(i) ) {
			//texcoords
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT2, 0 ));
		}
		
		if( modelSource->hasSkeleton(i) ) {
			//boneweights
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT4, 0 ));
			//boneindices
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT4, 0 ));
		}
		
		section->setVboMesh( modelSource->getNumVertices(i), modelSource->getNumIndices(i), layout, GL_TRIANGLES );
		
		section->setSkeleton( skeleton );
		mMeshSections.push_back( section );
	}
	mActiveSection = mMeshSections[0];
	
	ModelTargetSkinnedVboMesh target( this );
	modelSource->load( &target );
}

MeshVboSectionRef& SkinnedVboMesh::setActiveSection( int index )
{
	assert( index >= 0 && index < (int) mMeshSections.size() );
	mActiveSection = mMeshSections[ index ];
	return mActiveSection;
}

void SkinnedVboMesh::update()
{
	for( MeshVboSectionRef section : mMeshSections ) {
		section->updateMesh( mEnableSkinning );
	}
}

} //end namespace model
