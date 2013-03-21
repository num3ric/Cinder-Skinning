
#include "ModelTargetSkinnedMesh.h"
#include "SkinnedMesh.h"
#include "Skeleton.h"

namespace model {

ModelTargetSkinnedMesh::ModelTargetSkinnedMesh( SkinnedMesh *mesh )
: mSkinnedMesh( mesh )
{ }

std::shared_ptr<Skeleton> ModelTargetSkinnedMesh::getSkeleton() const
{
	return mSkinnedMesh->getSkeleton();
}

void ModelTargetSkinnedMesh::setActiveSection( int index )
{
	mSkinnedMesh->setActiveSection( index );
}

void ModelTargetSkinnedMesh::loadName( std::string name )
{
	mSkinnedMesh->getActiveSection()->mName = name;
}

void ModelTargetSkinnedMesh::loadVertexPositions( const std::vector<ci::Vec3f> &positions )
{
	mSkinnedMesh->getActiveSection()->mInitialPositions = positions;
	mSkinnedMesh->getActiveSection()->mTriMesh.appendVertices( positions.data(), positions.size() );
}

void ModelTargetSkinnedMesh::loadIndices( const std::vector<uint32_t>& indices )
{
	//TODO: Had to change Trimesh to pass const argument. Make pull request.
	mSkinnedMesh->getActiveSection()->mTriMesh.appendIndices( indices.data(), indices.size() );
}

void ModelTargetSkinnedMesh::loadTex( const std::vector<ci::Vec2f>& texCoords, const MaterialInfo& matInfo )
{
	mSkinnedMesh->getActiveSection()->mTriMesh.appendTexCoords( texCoords.data(), texCoords.size() );
	mSkinnedMesh->getActiveSection()->setMatInfo( matInfo );
}

void ModelTargetSkinnedMesh::loadVertexNormals( const std::vector<ci::Vec3f>& normals )
{
	mSkinnedMesh->getActiveSection()->mInitialNormals = normals;
	mSkinnedMesh->getActiveSection()->mTriMesh.appendNormals( normals.data(), normals.size() );
	mSkinnedMesh->getActiveSection()->setHasNormals( true ); //FIXME: remove this
}

void ModelTargetSkinnedMesh::loadSkeleton( const SkeletonRef& skeleton )
{	
	mSkinnedMesh->getActiveSection()->setSkeleton( skeleton );
}

void ModelTargetSkinnedMesh::loadBoneWeights( const std::vector<BoneWeights>& boneWeights )
{
	mSkinnedMesh->getActiveSection()->setBoneWeights( boneWeights );
}

void ModelTargetSkinnedMesh::loadDefaultTransformation( const ci::Matrix44f& transformation )
{
	mSkinnedMesh->setDefaultTransformation( transformation );
}

} //end namespace model
