#include "ModelTargetSkinnedVboMesh.h"

#include "Skeleton.h"
#include "SkinnedVboMesh.h"

namespace model {

ModelTargetSkinnedVboMesh::ModelTargetSkinnedVboMesh( SkinnedVboMesh * mesh )
: mSkinnedVboMesh( mesh )
, mOffset(0)
{
	
}

template<class T>
void ModelTargetSkinnedVboMesh::bufferSubData( const T& buffer, size_t dim )
{
	size_t dataSize = sizeof(GLfloat) * dim * buffer.size();
	ci::gl::VboMesh& vboMesh = mSkinnedVboMesh->getActiveSection()->getVboMesh();
	vboMesh.getStaticVbo().bufferSubData(mOffset, dataSize, buffer.data());
	vboMesh.getStaticVbo().unbind();
	mOffset += dataSize;
}

void ModelTargetSkinnedVboMesh::setCustomAttribute( const std::string& name, int location )
{
	mSkinnedVboMesh->getActiveSection()->getShader().bind();
	mSkinnedVboMesh->getActiveSection()->getVboMesh().setCustomStaticLocation( location, mSkinnedVboMesh->getActiveSection()->getShader().getAttribLocation( name ) );
	mSkinnedVboMesh->getActiveSection()->getShader().unbind();
}

void ModelTargetSkinnedVboMesh::setActiveSection( int index )
{
	mSkinnedVboMesh->setActiveSection( index );
	mOffset = 0;
}

std::shared_ptr<Skeleton> ModelTargetSkinnedVboMesh::getSkeleton() const
{
	return mSkinnedVboMesh->getSkeleton();
}

void ModelTargetSkinnedVboMesh::loadVertexPositions( const std::vector<ci::Vec3f>& positions )
{
	bufferSubData< std::vector<ci::Vec3f> >( positions, ci::Vec3f::DIM );
	setCustomAttribute( "position", 0 );
}

void ModelTargetSkinnedVboMesh::loadVertexNormals( const std::vector<ci::Vec3f>& normals )
{
	mSkinnedVboMesh->getActiveSection()->setHasNormals( true ); //FIXME: remove this
		
	bufferSubData< std::vector<ci::Vec3f> >( normals, ci::Vec3f::DIM );
	setCustomAttribute( "normal", 1 );
}

void ModelTargetSkinnedVboMesh::loadIndices( const std::vector<uint32_t>& indices )
{
	ci::gl::VboMesh& vboMesh = mSkinnedVboMesh->getActiveSection()->getVboMesh();
	vboMesh.bufferIndices( indices );
	vboMesh.unbindBuffers();
}

void ModelTargetSkinnedVboMesh::loadTex( const std::vector<ci::Vec2f>& texCoords, const MaterialInfo& matInfo )
{
	mSkinnedVboMesh->getActiveSection()->setMatInfo( matInfo );
		
	bufferSubData< std::vector<ci::Vec2f> >( texCoords, ci::Vec2f::DIM );
	setCustomAttribute( "texcoord", 2 );
}

void ModelTargetSkinnedVboMesh::loadSkeleton( const SkeletonRef& skeleton )
{
	if( skeleton->getNumBones() > SkinnedVboMesh::MAXBONES )
		throw "Skeleton has more bones than maximal the number allowed.";
	
	mSkinnedVboMesh->setSkeleton( skeleton );
}

void ModelTargetSkinnedVboMesh::loadBoneWeights( const std::vector<BoneWeights>& boneWeights )
{
	std::vector<ci::Vec4f> boneWeightsBuffer;
	std::vector<ci::Vec4f> boneIndicesBuffer;
	
	for( const auto& boneWeight : boneWeights ) {
		ci::Vec4f vWeights = ci::Vec4f::zero();
		ci::Vec4f vIndices = ci::Vec4i::zero();
		for( unsigned int b =0; b < boneWeight.mActiveNbWeights; ++b ) {
			NodeRef bone = boneWeight.getBone( b );
			vWeights[b] = boneWeight.getWeight(b);
			//FIXME: Maybe use ints on the desktop?
			vIndices[b] = (float) mSkinnedVboMesh->getSkeleton()->getBoneIndex( bone->getName() );
		}
		boneWeightsBuffer.push_back( vWeights );
		boneIndicesBuffer.push_back( vIndices );
	}
	
	bufferSubData< std::vector<ci::Vec4f> >( boneWeightsBuffer, ci::Vec4f::DIM );
	bufferSubData< std::vector<ci::Vec4f> >( boneIndicesBuffer, ci::Vec4f::DIM );
	setCustomAttribute( "boneWeights", 3 );
	setCustomAttribute( "boneIndices", 4 );
	
	mSkinnedVboMesh->getActiveSection()->boneMatrices = &mSkinnedVboMesh->mBoneMatrices;
	mSkinnedVboMesh->getActiveSection()->invTransposeMatrices = &mSkinnedVboMesh->mInvTransposeMatrices;
}
	
void ModelTargetSkinnedVboMesh::loadDefaultTransformation( const ci::Matrix44f& transformation )
{
	mSkinnedVboMesh->setDefaultTransformation( transformation );
}

} //end namespace model
