#pragma once

#include "ModelIo.h"
#include <vector>

#include "cinder/Vector.h"

class Skeleton;
class SkinnedVboMesh;

using namespace ci;

class ModelTargetSkinnedVboMesh : public ModelTarget {
public:	
	ModelTargetSkinnedVboMesh( SkinnedVboMesh *mesh );
	
	virtual void	setActiveSection( int index ) override;
	virtual std::shared_ptr<Skeleton> getSkeleton() const override;
	
	virtual void	loadVertexPositions( const std::vector<Vec3f>& positions ) override;
	virtual void	loadVertexNormals( const std::vector<Vec3f>& normals ) override;
	virtual void	loadIndices( const std::vector<uint32_t>& indices ) override;
	virtual void	loadTex( const std::vector<Vec2f>& texCoords, const MaterialInfo& matInfo ) override;
  	virtual void	loadSkeleton( const std::shared_ptr<Skeleton>& skeleton ) override;
	virtual void	loadBoneWeights( const std::vector<BoneWeights>& boneWeights ) override;  	
private:
  	SkinnedVboMesh*			mSkinnedVboMesh;
	
	template<class T> void	bufferSubData( const T& buffer, size_t dim);
	void					setCustomAttribute( const std::string& name, int location );
	
	int mOffset;
};