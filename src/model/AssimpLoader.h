#pragma once

#include "assimp/types.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/Importer.hpp"

#include "ModelIo.h"

#include <boost/noncopyable.hpp>

namespace model {
	
	class SurfacePool;
		
	namespace ai {
		//! Assimp loader settings/flags.
		extern const unsigned int FLAGS;
		extern const unsigned int MORPHTARGETS_FLAGS;
		
		//! Convert aiVector3D to glm::vec3.
		inline glm::vec3				get( const aiVector3D &v );
		//! Convert aiQuaternion to ci::quat.
		inline ci::quat				get( const aiQuaternion &q );
		//! Convert aiMatrix4x4 to glm::mat4.
		inline glm::mat4			get( const aiMatrix4x4 &m );
		//! Convert aiColor4D to ci::ColorAf.
		inline ci::ColorAf				get( const aiColor4D &c );
		//! Convert aiString to std::string.
		inline std::string				get( const aiString &s );
		//! Extract vertex positions from an assimp mesh section.
		std::vector<glm::vec3>			getPositions( const aiMesh* aimesh );
		//! Extract vertex normals from an assimp mesh section.
		std::vector<glm::vec3>			getNormals( const aiMesh* aimesh );
		//! Extract vertex tangents from an assimp mesh section.
		std::vector<glm::vec3>			getTangents( const aiMesh* aimesh );
		//! Extract vertex bitangents from an assimp mesh section.
		std::vector<glm::vec3>			getBitangents( const aiMesh* aimesh );
		//! Extract vertex texture coordinates from an assimp mesh section.
		std::vector<glm::vec2>			getTexCoords( const aiMesh* aimesh, unsigned int unit = 0 );
		//! Extract vertex indices from an assimp mesh section.
		std::vector<uint32_t>			getIndices( const aiMesh* aimesh );
		//! Extract material information (including textures) for a mesh section.
		model::MaterialSource			getMaterial( const aiScene* aiscene, const aiMesh *aimesh, ci::fs::path modelPath, const std::shared_ptr<SurfacePool>& surfacePool, ci::fs::path rootPath = ""  );
		//! Extract skeletal bone weights for each vertex of an assimp mesh section.
		std::vector<model::Weights>		getBoneWeights( const aiMesh* aimesh, const model::Skeleton* skeleton );
		//! Extract a mesh section's default transformation (use when there is no bones)
		glm::mat4					getDefaultTransformation( const std::string& name, const aiScene* aiscene, model::Skeleton* skeleton );
		
		//! Construct skeleton from assimp scene.
		std::shared_ptr<class model::Skeleton>	getSkeleton( const aiScene* aiscene,
															const aiNode* root = nullptr );
		//! Traverse assimp nodes to find the aiNode with specified name.
		const aiNode*							findMeshNode( const std::string& meshName,
															 const aiScene* aiscene,
															 const aiNode* ainode );
	}
	
	class AssimpLoader : public model::Source, public boost::noncopyable {
	public:
		struct Settings {
			//TODO: toggle loading lights, cameras, meshes, animations, textures & flags
			Settings() : mLoadAnims(true), mFlags(ai::FLAGS) { }
			
			Settings& assimpFlags( unsigned int flags ) { mFlags = flags; return *this; }
			
			Settings& morphTargets( const std::vector<ci::DataSourceRef>& targets ) { mMorphTargets = targets; mFlags = ai::MORPHTARGETS_FLAGS; return *this; }
			Settings& rootFolder( const ci::fs::path& rootAssetFolderPath ) { mRootAssetFolderPath = rootAssetFolderPath; return *this; }
			Settings& loadAnims( bool loadAnims ) { mLoadAnims = loadAnims; return *this; }
			Settings& surfaces( const std::shared_ptr<SurfacePool>& surfacePool ) { mSurfacePool = surfacePool; return *this; }
		private:
			bool mLoadAnims;
			unsigned int mFlags;
			
			std::shared_ptr<SurfacePool> mSurfacePool;
			std::vector<ci::DataSourceRef> mMorphTargets;
			ci::fs::path mRootAssetFolderPath;
			
			friend class AssimpLoader;
		};
		
		explicit AssimpLoader( const ci::DataSourceRef& modelSource, const Settings& settings = Settings()  );
				
		const std::shared_ptr<SurfacePool>&			getSurfacePool() { return mSurfacePool; }
	protected:
		const aiScene*		loadAiScene( const ci::DataSourceRef& dataSource, Assimp::Importer* importer, unsigned int flags );
		void				loadScene( const aiScene* aiScene );
		void				loadMorphTarget( const aiScene* aiscene );

		bool mHasAnimations, mHasSkeleton;
		//! File path to the model.
		ci::fs::path					mModelPath;
		//! Root asset folder (textures in a model may not reside in the same directory as the model).
		ci::fs::path					mRootAssetFolderPath;

		std::shared_ptr<SurfacePool>	mSurfacePool;
	};
	
} //end namespace model
