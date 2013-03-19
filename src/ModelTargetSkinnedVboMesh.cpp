#include "ModelTargetSkinnedVboMesh.h"

#include "Skeleton.h"
#include "SkinnedVboMesh.h"

ModelTargetSkinnedVboMesh::ModelTargetSkinnedVboMesh( SkinnedVboMesh * mesh )
: mSkinnedVboMesh( mesh )
, mOffset(0)
{
	
}

template<class T>
void ModelTargetSkinnedVboMesh::bufferSubData( const T& buffer, size_t dim )
{
	size_t dataSize = sizeof(GLfloat) * dim * buffer.size();
	gl::VboMesh& vboMesh = mSkinnedVboMesh->getActiveSection()->getVboMesh();
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

void ModelTargetSkinnedVboMesh::loadVertexPositions( const std::vector<Vec3f>& positions )
{
	bufferSubData< std::vector<Vec3f> >( positions, Vec3f::DIM );
	setCustomAttribute( "position", 0 );
}

void ModelTargetSkinnedVboMesh::loadVertexNormals( const std::vector<Vec3f>& normals )
{
	mSkinnedVboMesh->getActiveSection()->setHasNormals( true ); //FIXME: remove this
		
	bufferSubData< std::vector<Vec3f> >( normals, Vec3f::DIM );
	setCustomAttribute( "normal", 1 );
}

void ModelTargetSkinnedVboMesh::loadIndices( const std::vector<uint32_t>& indices )
{
	gl::VboMesh& vboMesh = mSkinnedVboMesh->getActiveSection()->getVboMesh();
	vboMesh.bufferIndices( indices );
	vboMesh.unbindBuffers();
}

void ModelTargetSkinnedVboMesh::loadTex( const std::vector<Vec2f>& texCoords, const MaterialInfo& matInfo )
{
	mSkinnedVboMesh->getActiveSection()->setMatInfo( matInfo );
		
	bufferSubData< std::vector<Vec2f> >( texCoords, Vec2f::DIM );
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
	std::vector<Vec4f> boneWeightsBuffer;
	std::vector<Vec4f> boneIndicesBuffer;
	
	for( const auto& boneWeight : boneWeights ) {
		Vec4f vWeights = Vec4f::zero();
		Vec4f vIndices = Vec4i::zero();
		for( int b =0; b < boneWeight.mActiveNbWeights; ++b ) {
			NodeRef bone = boneWeight.getBone( b );
			vWeights[b] = boneWeight.getWeight(b);
			vIndices[b] = mSkinnedVboMesh->getSkeleton()->getBoneIndex( bone->getName() );
		}
		boneWeightsBuffer.push_back( vWeights );
		boneIndicesBuffer.push_back( vIndices );
	}
	
	bufferSubData< std::vector<Vec4f> >( boneWeightsBuffer, Vec4f::DIM );
	bufferSubData< std::vector<Vec4f> >( boneIndicesBuffer, Vec4f::DIM );
	setCustomAttribute( "boneWeights", 3 );
	setCustomAttribute( "boneIndices", 4 );
	
	mSkinnedVboMesh->getActiveSection()->boneMatrices = &mSkinnedVboMesh->mBoneMatrices;
	mSkinnedVboMesh->getActiveSection()->invTransposeMatrices = &mSkinnedVboMesh->mInvTransposeMatrices;
}