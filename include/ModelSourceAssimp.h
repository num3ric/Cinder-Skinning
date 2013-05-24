#pragma once

#include "ModelIo.h"

#include "assimp/types.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
//#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"	//OO version Header!

#include <unordered_set>

class model::Node;

namespace ai {
	
	extern unsigned int flags;
	
	//! Convert aiVector3D to ci::Vec3f.
	inline ci::Vec3f get( const aiVector3D &v ) {
		return ci::Vec3f( v.x, v.y, v.z );
	}
	
	//! Convert aiQuaternion to ci::Quatf.
	inline ci::Quatf get( const aiQuaternion &q ) {
		return ci::Quatf( q.w, q.x, q.y, q.z );
	}
	
	//! Convert aiMatrix4x4 to ci::Matrix44f.
	inline ci::Matrix44f get( const aiMatrix4x4 &m ) {
		return ci::Matrix44f( &m.a1, true );
	}
	
	//! Convert aiColor4D to ci::ColorAf.
	inline ci::ColorAf get( const aiColor4D &c ) {
		return ci::ColorAf( c.r, c.g, c.b, c.a );
	}
	
	//! Convert aiString to std::string.
	inline std::string get( const aiString &s ) {
		return std::string( s.data );
	}
	
	const aiNode* findMeshNode( const std::string& meshName, const aiScene* aiscene, const aiNode* ainode );
	
	//FIXME: Move into private namespace?
	std::shared_ptr<class model::Skeleton> loadSkeleton( bool hasAnimations, const aiScene* aiscene, const aiNode* root = nullptr );
	
	std::shared_ptr<model::Node> generateNodeHierarchy(model::Skeleton* skeleton,
												const aiNode* ainode,
												const std::shared_ptr<model::Node>& parent = nullptr,
												ci::Matrix44f derivedTransformation = ci::Matrix44f::identity(),
												int level = 0 );
	
	void generateAnimationCurves( model::Skeleton* skeleton, const aiScene* aiscene );
	
	void loadPositions( const aiMesh* aimesh, std::vector<ci::Vec3f>* positions );

	void loadNormals( const aiMesh* aimesh, std::vector<ci::Vec3f>* normals );

	void loadTexCoords( const aiMesh* aimesh, std::vector<ci::Vec2f>* texCoords );

	void loadIndices( const aiMesh* aimesh, std::vector<uint32_t>* indices );
	
	void loadTexture( const aiScene* aiscene, const aiMesh *aimesh, model::MaterialInfo* texInfo, ci::fs::path modelPath, ci::fs::path rootPath = ""  );
	
	void loadBoneWeights( const aiMesh* aimesh, const model::Skeleton* skeleton, std::vector<model::BoneWeights>* boneWeights  );
	
	//TODO: Use when there is no bones
	ci::Matrix44f getDefaultTransformation( const std::string& name, const aiScene* aiscene, model::Skeleton* skeleton );

}



namespace model {

typedef std::shared_ptr< class ModelSourceAssimp > ModelSourceAssimpRef;

class ModelSourceAssimp : public ModelSource {
	
	struct ModelInfo {
		ModelInfo()
		:mHasNormals( false )
		, mHasSkeleton( false )
		, mHasAnimations( false )
		, mNumSections( 1 )
		{ }
		bool mHasNormals;
		bool mHasSkeleton;
		bool mHasAnimations;
		std::vector<bool> mHasMaterials;
		std::vector<size_t> mNumVertices;
		std::vector<size_t> mNumIndices;
		size_t mNumSections;
	};
public:
//	static ModelSourceRef	create( DataSourceRef dataSource );
	static ModelSourceAssimpRef	create( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath = ""  );
	
	virtual size_t	getNumSections() const override { return mModelInfo.mNumSections; }
	virtual size_t	getNumVertices( int section = -1 ) const override;
	virtual size_t	getNumIndices( int section = -1 )  const override;
	//TODO: Maybe the following functions should also have a per section argument
	virtual bool	hasNormals() const override { return mModelInfo.mHasNormals; }
  	virtual bool	hasSkeleton() const override { return mModelInfo.mHasSkeleton; }
	virtual bool	hasAnimations() const override { return mModelInfo.mHasAnimations; }
	virtual bool	hasMaterials( int section = -1 ) const override;
	
	virtual void	load( ModelTarget *target ) override;
	
protected:
	ModelSourceAssimp( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath = ""  );
private:
	//! Assimp importer instance which cannot be destroyed until the scene loading is complete.
	std::unique_ptr<Assimp::Importer>	mImporter;
	//! Assimp scene pointer which is parsed and loaded into cinder.
	const aiScene*						mAiScene;
	
	//! File path to the model.
	ci::fs::path						mModelPath;
	//! Root asset folder (textures in a model may not reside in the same directory as the model).
	ci::fs::path						mRootAssetFolderPath;

	ModelInfo mModelInfo;
};

} //end namespace model
