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
#include "Resources.h"

namespace model {

SkinnedVboMesh::MeshSection::MeshSection()
: ASkinnedMesh()
{
	try {
		mSkinningShader = ci::gl::GlslProg( ci::app::loadResource(RES_SKINNING_VERT), ci::app::loadResource(RES_SKINNING_FRAG) );
	}
	catch( ci::gl::GlslProgCompileExc &exc ) {
		std::cout << "Shader compile error: " << std::endl;
		std::cout << exc.what();
	}
}

void SkinnedVboMesh::MeshSection::setVboMesh( size_t numVertices, size_t numIndices, ci::gl::VboMesh::Layout layout, GLenum primitiveType )
{
	mVboMesh = ci::gl::VboMesh( numVertices, numIndices, layout, primitiveType );
}

void SkinnedVboMesh::MeshSection::updateMesh( float time, bool enableSkinning )
{
	if (enableSkinning) {
		int i = 0;
		for( const auto& entry : mSkeleton->getBoneNames() ) {
			if( i >= MAXBONES )
				break;
			NodeRef bone = entry.second;
			(*boneMatrices)[i] = bone->getAbsoluteTransformation() * *bone->getOffset();
			(*invTransposeMatrices)[i] = (*boneMatrices)[i].orthonormalInverted();
			(*invTransposeMatrices)[i].transpose();
			++i;
		}
		mIsAnimated = true;
	} else if( mIsAnimated ) {
		mIsAnimated = false;
	}
}

void SkinnedVboMesh::MeshSection::drawMesh()
{
    mSkinningShader.bind();
	mSkinningShader.uniform( "isAnimated", mIsAnimated );
    mSkinningShader.uniform( "texture", 0 );
    mSkinningShader.uniform( "boneMatrices", boneMatrices->data(), SkinnedVboMesh::MAXBONES );
	mSkinningShader.uniform( "invTransposeMatrices", invTransposeMatrices->data(), SkinnedVboMesh::MAXBONES );
    ci::gl::draw( mVboMesh );
	//    ci::gl::drawRange(mVbo, 0, mVbo.getNumIndices()*3);
    mSkinningShader.unbind();
}

SkinnedVboMeshRef SkinnedVboMesh::create( ModelSourceRef modelSource, const SkeletonRef& skeleton )
{
	return SkinnedVboMeshRef( new SkinnedVboMesh( modelSource, skeleton ) );
}

SkinnedVboMesh::SkinnedVboMesh( ModelSourceRef modelSource, const SkeletonRef& skeleton )
{
	assert( modelSource->getNumSections() > 0 );
	
	for(int i = 0; i< modelSource->getNumSections(); ++i ) {
		MeshVboSectionRef section = std::make_shared<SkinnedVboMesh::MeshSection>();
		ci::gl::VboMesh::Layout layout;
		layout.setStaticIndices();
		//positions
		layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT3, 0 ));
		
		if( modelSource->hasNormals() ) {
			//normals
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT3, 0 ));
		}
		
		if( modelSource->hasMaterials() ) {
			//texcoords
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT2, 0 ));
		}
		
		if( modelSource->hasSkeleton() ) {
			//boneweights
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT4, 0 ));
			//boneindices
			layout.mCustomStatic.push_back(std::make_pair(ci::gl::VboMesh::Layout::CUSTOM_ATTR_FLOAT4, 0 ));
		}
		
		section->setVboMesh( modelSource->getNumVertices(i), modelSource->getNumIndices(i), layout, GL_TRIANGLES );
		
		mMeshSections.push_back( section );
	}
	mActiveSection = mMeshSections[0];
	
	ModelTargetSkinnedVboMesh target( this );
	modelSource->load( &target );
}

MeshVboSectionRef& SkinnedVboMesh::setActiveSection( int index )
{
	assert( index >= 0 && index < mMeshSections.size() );
	mActiveSection = mMeshSections[ index ];
	return mActiveSection;
}

void SkinnedVboMesh::update( float time, bool enableSkinning )
{
	mActiveSection->getSkeleton()->update( time );
	for( MeshVboSectionRef section : mMeshSections ) {
		section->updateMesh( time, enableSkinning );
	}
}

void SkinnedVboMesh::draw()
{
	for( const MeshVboSectionRef& section : mMeshSections ) {
		section->draw();
	}
}

} //end namespace model
