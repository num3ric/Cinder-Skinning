#pragma once

#include "cinder/Vector.h"
#include "cinder/DataSource.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Material.h"

#include <array>

class Skeleton;
class Node;
class ModelTarget;

typedef std::shared_ptr<class ModelSource> ModelSourceRef;
typedef std::shared_ptr<ModelTarget> ModelTargetRef;

using namespace ci;

struct MaterialInfo
{
	gl::Texture		mTexture;
	gl::Material	mMaterial;
	Color			mTransparentColor = Color::white();
	bool			mUseAlpha = false;
	bool			mHasMaterial = false;
	bool			mTwoSided = false;
};

class BoneWeights {
public:
	static const int NB_WEIGHTS = 4;
	void			addWeight( const std::shared_ptr<Node>& bone, float weight );
	float			getWeight( int index ) const  { return mWeights[index]; }
	const std::shared_ptr<Node>&	getBone( int index ) const { return mBones[index]; }
	size_t		mActiveNbWeights = 0;
private:
	std::array<float, NB_WEIGHTS> mWeights;
	std::array<std::shared_ptr<Node>, NB_WEIGHTS> mBones;
};

class ModelSource {
public:
	virtual size_t	getNumSections() const = 0;
	virtual size_t	getNumVertices( int section = -1 ) const = 0;
	virtual size_t	getNumIndices( int section = -1 )  const = 0;
	virtual bool	hasNormals() const = 0;
  	virtual bool	hasSkeleton() const = 0;
	virtual bool	hasAnimations() const = 0;
	virtual bool	hasMaterials() const = 0;
	
  	virtual	void	load( ModelTarget *target ) = 0;
};

//! Load model with assimp, the only loader we have for now.
extern std::shared_ptr<class ModelSourceAssimp>	loadModel( const fs::path& modelPath, const fs::path& rootAssetFolderPath = "" )
;

class ModelTarget {
public:
	virtual void	setActiveSection( int index );
	virtual std::shared_ptr<Skeleton> getSkeleton() const = 0;
	
	virtual void	loadName( std::string name );
	virtual void	loadVertexPositions( const std::vector<Vec3f>& positions ) = 0;
	virtual void	loadIndices( const std::vector<uint32_t>& indices ) = 0;
	virtual void	loadTex( const std::vector<Vec2f>& texCoords, const MaterialInfo& texInfo );
  	virtual void	loadVertexNormals( const std::vector<Vec3f>& normals );
  	virtual void	loadSkeleton( const std::shared_ptr<Skeleton>& skeleton );
	virtual void	loadBoneWeights( const std::vector<BoneWeights>& boneWeights );	
};
