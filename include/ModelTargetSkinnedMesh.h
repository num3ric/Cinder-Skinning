#pragma once

#include "ModelIo.h"

#include "cinder/Vector.h"

class Skeleton;
class SkinnedMesh;

using namespace ci;

class ModelTargetSkinnedMesh : public ModelTarget {
public:
	ModelTargetSkinnedMesh( SkinnedMesh *mesh );
	virtual void	setActiveSection( int index ) override;
	std::shared_ptr<Skeleton> getSkeleton() const override;
	
	virtual void	loadName( std::string name ) override;
	virtual void	loadVertexPositions( const std::vector<Vec3f>& positions ) override;
	virtual void	loadIndices( const std::vector<uint32_t>& indices ) override;
	virtual void	loadTex( const std::vector<Vec2f>& texCoords, const MaterialInfo& matInfo )  override;
  	virtual void	loadVertexNormals( const std::vector<Vec3f>& normals )  override;
  	virtual void	loadSkeleton( const std::shared_ptr<Skeleton>& skeleton )  override;
	virtual void	loadBoneWeights( const std::vector<BoneWeights>& boneWeights )  override;	
private:
  	SkinnedMesh*	mSkinnedMesh;
};