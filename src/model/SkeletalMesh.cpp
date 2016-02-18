#include "SkeletalMesh.h"
#include "ModelIo.h"

#include "Skeleton.h"
#include "Renderer.h"

#include "cinder/CinderAssert.h"

#include "glm/gtc/matrix_inverse.hpp"

using namespace ci;
using namespace model;

SkeletalMesh::Section::Section( const SectionSourceRef& source,  gl::GlslProgRef shader )
: ABatchSection( source, shader )
{
}

SkeletalMeshRef SkeletalMesh::create( const model::Source& modelSource, SkeletonRef skeleton, gl::GlslProgRef skinningShader )
{
	if( ! skinningShader )
		skinningShader = model::Renderer::instance().getShader( MeshType::SKELETAL );

	return SkeletalMeshRef( new SkeletalMesh( modelSource, skeleton, skinningShader ) );
}

SkeletalMesh::SkeletalMesh( const model::Source& modelSource, SkeletonRef skeleton, gl::GlslProgRef skinningShader )
: Actor( modelSource, skeleton )
{
	for( auto& sectionSource : modelSource.getSectionSources() ) {
		auto section = SectionRef( new Section{ sectionSource, skinningShader } );
		section->mSkeleton				= mSkeleton;
		section->mInvTransposeMatrices	= &mInvTransposeMatrices;
		section->mBoneMatrices			= &mBoneMatrices;
		
		mMeshSections.push_back( section );
	}
}

void SkeletalMesh::update()
{
	int i = 0;
	for( const auto& entry : mSkeleton->getBones() ) {
		if( i >= MAXBONES ) {
			CI_ASSERT_MSG( false, "Skeleton exceeds maximum supported number of bones." );
			break;
		}
		const NodeRef& bone = entry.second;
		size_t idx = bone->getBoneIndex();
		( mBoneMatrices ).at( idx ) = bone->getAbsoluteTransformation() * *bone->getOffset();
		( mInvTransposeMatrices ).at( idx ) = glm::inverseTranspose( mBoneMatrices.at( idx ) );
		++i;
	}
}

