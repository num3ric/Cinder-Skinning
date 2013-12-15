#include "ModelTargetSkinnedVboMesh.h"

#include "Skeleton.h"
#include "SkinnedVboMesh.h"

namespace model {

ModelTargetSkinnedVboMesh::ModelTargetSkinnedVboMesh( SkinnedVboMesh * mesh )
: mSkinnedVboMesh( mesh )
, mSubDataOffset(0)
, mAttribLocation(0)
{
	
}

void ModelTargetSkinnedVboMesh::incrementOffsets( size_t dataSize )
{
	mSubDataOffset += dataSize;
	mAttribLocation++;
}

template<class T>
void ModelTargetSkinnedVboMesh::bufferSubData( const T& buffer, size_t dataSize )
{
	ci::gl::VboMesh& vboMesh = mSkinnedVboMesh->getActiveSection()->getVboMesh();
	vboMesh.getStaticVbo().bufferSubData( mSubDataOffset, dataSize, buffer.data() );
	vboMesh.getStaticVbo().unbind();
}

void ModelTargetSkinnedVboMesh::setCustomAttribute( GLuint location, const std::string& name )
{
	MeshVboSectionRef sect = mSkinnedVboMesh->getActiveSection();
	mSkinnedVboMesh->getShader()->bind();
	GLint attribLocation = mSkinnedVboMesh->getShader()->getAttribLocation( name );

	if( attribLocation < 0 )
		throw ModelTargetException( "Discrepancy between what the model target expects and what its shader attributes use." );

	sect->getVboMesh().setCustomStaticLocation( location, attribLocation );
	mSkinnedVboMesh->getShader()->unbind();
}

void ModelTargetSkinnedVboMesh::setActiveSection( int index )
{
	mSkinnedVboMesh->setActiveSection( index );
	mSubDataOffset = 0;
	mAttribLocation = 0;
}

std::shared_ptr<Skeleton> ModelTargetSkinnedVboMesh::getSkeleton() const
{
	return mSkinnedVboMesh->getSkeleton();
}

void ModelTargetSkinnedVboMesh::loadIndices( const std::vector<uint32_t>& indices )
{
	ci::gl::VboMesh& vboMesh = mSkinnedVboMesh->getActiveSection()->getVboMesh();
	vboMesh.bufferIndices( indices );
	vboMesh.unbindBuffers();
}

void ModelTargetSkinnedVboMesh::loadVertexPositions( const std::vector<ci::Vec3f>& positions )
{
	size_t dataSize = sizeof(GLfloat) * ci::Vec3f::DIM * positions.size();
	bufferSubData< std::vector<ci::Vec3f> >( positions, dataSize );
	setCustomAttribute( mAttribLocation, "position" );
	incrementOffsets( dataSize );
}

void ModelTargetSkinnedVboMesh::loadVertexNormals( const std::vector<ci::Vec3f>& normals )
{
	mSkinnedVboMesh->getActiveSection()->setHasNormals( true ); //FIXME: remove this
	
	size_t dataSize = sizeof(GLfloat) * ci::Vec3f::DIM * normals.size();
	bufferSubData< std::vector<ci::Vec3f> >( normals, dataSize );
	setCustomAttribute( mAttribLocation, "normal" );
	incrementOffsets( dataSize );
}

void ModelTargetSkinnedVboMesh::loadTex( const std::vector<ci::Vec2f>& texCoords, const MaterialInfo& matInfo )
{
	mSkinnedVboMesh->getActiveSection()->setMatInfo( matInfo );
	
	size_t dataSize = sizeof(GLfloat) * ci::Vec2f::DIM * texCoords.size();
	bufferSubData< std::vector<ci::Vec2f> >( texCoords, dataSize);
	setCustomAttribute( mAttribLocation, "texcoord" );
	incrementOffsets( dataSize );
}

void ModelTargetSkinnedVboMesh::loadSkeleton( const SkeletonRef& skeleton )
{
	if( skeleton->getNumBones() > SkinnedVboMesh::MAXBONES )
		throw ModelTargetException( "Skeleton has more bones than maximal the number allowed." );
	
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
			vIndices[b] = bone->getBoneIndex();
		}
		boneWeightsBuffer.push_back( vWeights );
		boneIndicesBuffer.push_back( vIndices );
	}
	
	size_t dataSize = sizeof(GLfloat) * ci::Vec4f::DIM * boneWeightsBuffer.size();
	
	bufferSubData< std::vector<ci::Vec4f> >( boneWeightsBuffer, dataSize );
	setCustomAttribute( mAttribLocation, "boneWeights" );
	incrementOffsets( dataSize );
	
	bufferSubData< std::vector<ci::Vec4f> >( boneIndicesBuffer, dataSize);
	setCustomAttribute( mAttribLocation, "boneIndices" );
	incrementOffsets( dataSize );
	
	mSkinnedVboMesh->getActiveSection()->mBoneMatricesPtr = &mSkinnedVboMesh->mBoneMatrices;
	mSkinnedVboMesh->getActiveSection()->mInvTransposeMatricesPtr = &mSkinnedVboMesh->mInvTransposeMatrices;
}
	
void ModelTargetSkinnedVboMesh::loadDefaultTransformation( const ci::Matrix44f& transformation )
{
	mSkinnedVboMesh->setDefaultTransformation( transformation );
}

} //end namespace model
