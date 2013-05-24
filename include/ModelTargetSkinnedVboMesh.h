#pragma once

#include "ModelIo.h"
#include <vector>

#include "cinder/Vector.h"

namespace model {

class Skeleton;
class SkinnedVboMesh;

class ModelTargetSkinnedVboMesh : public ModelTarget {
public:	
	ModelTargetSkinnedVboMesh( SkinnedVboMesh *mesh );
	
	virtual void	setActiveSection( int index ) override;
	virtual std::shared_ptr<Skeleton> getSkeleton() const override;
	
	virtual void	loadVertexPositions( const std::vector<ci::Vec3f>& positions ) override;
	virtual void	loadVertexNormals( const std::vector<ci::Vec3f>& normals ) override;
	virtual void	loadIndices( const std::vector<uint32_t>& indices ) override;
	virtual void	loadTex( const std::vector<ci::Vec2f>& texCoords, const MaterialInfo& matInfo ) override;
  	virtual void	loadSkeleton( const std::shared_ptr<Skeleton>& skeleton ) override;
	virtual void	loadBoneWeights( const std::vector<BoneWeights>& boneWeights ) override;
 	virtual void	loadDefaultTransformation( const ci::Matrix44f& transformation ) override;
private:
  	SkinnedVboMesh*			mSkinnedVboMesh;
	
	void					incrementOffsets( size_t dataSize );
	void					resetOffsets();
	
	template<class T> void	bufferSubData( const T& buffer, size_t dataSize);
	void					setCustomAttribute( GLuint location, const std::string& name );
	
	GLuint			mAttribLocation;
	ptrdiff_t		mSubDataOffset;
};

} //end namespace model