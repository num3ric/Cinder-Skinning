#include "AssimpLoader.h"
#include "CustomIOStream.h"
#include "Skeleton.h"
#include "Debug.h"

#include "SurfacePool.h"

#include "assimp/postprocess.h"
#include "cinder/ImageIo.h"
#include "cinder/app/App.h"

#include <boost/algorithm/string.hpp>
#include <assert.h> 
#include <algorithm>

#include "glm/gtc/type_ptr.hpp"

using namespace glm;

namespace model {

namespace ai {
	const unsigned int FLAGS =
	aiProcess_Triangulate |
	aiProcess_FlipUVs |
	aiProcess_FixInfacingNormals |
	aiProcess_FindInstances |
	aiProcess_ValidateDataStructure |
	aiProcess_OptimizeMeshes |
	aiProcess_CalcTangentSpace |
	aiProcess_GenSmoothNormals |
	aiProcess_JoinIdenticalVertices |
	aiProcess_ImproveCacheLocality |
	aiProcess_LimitBoneWeights |
	aiProcess_RemoveRedundantMaterials |
	aiProcess_GenUVCoords |
	aiProcess_SortByPType |
	aiProcess_FindDegenerates |
	aiProcess_FindInvalidData ;
//	aiProcess_SplitLargeMeshes |
//	aiProcess_FindInstances |

	const unsigned int MORPHTARGETS_FLAGS =
		aiProcess_FlipUVs |
		aiProcess_FixInfacingNormals |
		aiProcess_ValidateDataStructure |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_GenUVCoords |
		aiProcess_SortByPType;
	
	vec3 get( const aiVector3D &v ) {
		return vec3( v.x, v.y, v.z );
	}
	
	ci::quat get( const aiQuaternion &q ) {
		return ci::quat( q.w, q.x, q.y, q.z );
	}
	
	mat4 get( const aiMatrix4x4 &m ) {
		return transpose( make_mat4( &m.a1 ) );
	}
	
	ci::ColorAf get( const aiColor4D &c ) {
		return ci::ColorAf( c.r, c.g, c.b, c.a );
	}
	
	std::string get( const aiString &s ) {
		return std::string( s.data );
	}
	
	std::vector<vec3> getPositions( const aiMesh* aimesh ) {
		if ( !aimesh->HasPositions() ) {
			return std::vector<vec3>();
		}
		
		std::vector<vec3> positions;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			positions.push_back( ai::get( aimesh->mVertices[i] ) );
		}
		return positions;
	}
	
	std::vector<vec3> getNormals( const aiMesh* aimesh ) {
		if ( !aimesh->HasNormals() ) {
			return std::vector<vec3>();
		}
		
		std::vector<vec3> normals;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			normals.push_back( ai::get( aimesh->mNormals[i] ) );
		}
		return normals;
	}
	
	std::vector<vec3> getTangents( const aiMesh* aimesh ) {
		if ( !aimesh->HasTangentsAndBitangents() ) {
			return std::vector<vec3>();
		}
		
		std::vector<vec3> tangents;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			tangents.push_back( ai::get( aimesh->mTangents[i] ) );
		}
		return tangents;
	}
	
	std::vector<vec3> getBiTangents( const aiMesh* aimesh ) {
		if ( !aimesh->HasTangentsAndBitangents() ) {
			return std::vector<vec3>();
		}
		
		std::vector<vec3> bitangents;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			bitangents.push_back( ai::get( aimesh->mBitangents[i] ) );
		}
		return bitangents;
	}
	
	std::vector<vec2> getTexCoords( const aiMesh* aimesh, unsigned int unit ) {
		if ( !aimesh->HasTextureCoords(unit) ) {
			return std::vector<vec2>();
		}
		
		std::vector<vec2> texCoords;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			texCoords.push_back( vec2(aimesh->mTextureCoords[unit][i].x,
										   aimesh->mTextureCoords[unit][i].y) );
		}
		assert( texCoords.size() > 0 );
		return texCoords;
	}
	
	std::vector<std::uint32_t> getIndices( const aiMesh* aimesh ) {
		std::vector<std::uint32_t> indices;
		for( unsigned int i=0; i < aimesh->mNumFaces; ++i ) {
			aiFace aiface = aimesh->mFaces[i];
			unsigned numIndices = aiface.mNumIndices;
			if( numIndices == 2 ) {
				indices.emplace_back( aiface.mIndices[0] );
				indices.emplace_back( aiface.mIndices[1] );
				indices.emplace_back( aiface.mIndices[1] );
			}
			else if( numIndices == 3 ) {
				indices.emplace_back( aiface.mIndices[0] );
				indices.emplace_back( aiface.mIndices[1] );
				indices.emplace_back( aiface.mIndices[2] );
			}
			else if( numIndices == 4 ) {
				indices.emplace_back( aiface.mIndices[0] );
				indices.emplace_back( aiface.mIndices[1] );
				indices.emplace_back( aiface.mIndices[2] );

				indices.emplace_back( aiface.mIndices[2] );
				indices.emplace_back( aiface.mIndices[3] );
				indices.emplace_back( aiface.mIndices[0] );
			}
			else {
				throw LoadErrorException( "Higher polygonal faces not supported not supported." );
			}
		}
		return indices;
	}
	
	void loadSurface( ci::fs::path texturePath, const aiMaterial *mtl, model::MaterialSource::TextureType type, const std::shared_ptr<SurfacePool>& surfacePool, model::MaterialSource* matSource )
	{
		int horizontalMapMode;
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_MAPPINGMODE_U_DIFFUSE( 0 ), horizontalMapMode ) ) {
			if( horizontalMapMode == aiTextureMapMode_Wrap ) {
				matSource->mWrapS = GL_REPEAT;
			} else if( horizontalMapMode == aiTextureMapMode_Clamp
					  || horizontalMapMode == aiTextureMapMode_Decal ) {
				matSource->mWrapS = GL_CLAMP_TO_EDGE;
			} else if( horizontalMapMode == aiTextureMapMode_Mirror ) {
				matSource->mWrapS = GL_MIRRORED_REPEAT;
			}
		}
		
		int verticalMapMode;
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_MAPPINGMODE_V_DIFFUSE( 0 ), verticalMapMode ) ) {
			if( verticalMapMode == aiTextureMapMode_Wrap ) {
				matSource->mWrapT = GL_REPEAT;
			} else if( verticalMapMode == aiTextureMapMode_Clamp
					  || verticalMapMode == aiTextureMapMode_Decal ) {
				matSource->mWrapT = GL_CLAMP_TO_EDGE;
			} else if( verticalMapMode == aiTextureMapMode_Mirror ) {
				matSource->mWrapT = GL_MIRRORED_REPEAT;
			}
		}
		
//		std::string ext = texturePath.extension().string();
//		boost::algorithm::to_lower( ext );
		
		matSource->mSurfaces[type] = surfacePool->loadSurface( texturePath );
	}
	
	
	model::MaterialSource getMaterial( const aiScene* aiscene, const aiMesh *aimesh, ci::fs::path modelPath, const std::shared_ptr<SurfacePool>& surfacePool, ci::fs::path rootPath )
	{
		model::MaterialSource matSource;
		// Handle material info
		aiMaterial *mtl = aiscene->mMaterials[ aimesh->mMaterialIndex ];
		
		//for( uint i=0; i<mtl->mNumProperties; ++i ) {
		//	ci::app::console() << ai::get( mtl->mProperties[i]->mKey ) << std::endl;
		//}
		
		aiString name;
		mtl->Get( AI_MATKEY_NAME, name );
		LOG_M << "material " << ai::get( name ) << std::endl;
		// Culling
		int twoSided;
		if ( ( AI_SUCCESS == mtl->Get( AI_MATKEY_TWOSIDED, twoSided ) ) && twoSided ) {
			matSource.mTwoSided = true;
			matSource.mMaterial.setFace( GL_FRONT_AND_BACK );
			LOG_M << " two sided" << std::endl;
		} else {
			matSource.mTwoSided = false;
			matSource.mMaterial.setFace( GL_FRONT );
		}
		
		aiColor4D dcolor, scolor, acolor, ecolor, tcolor;
		if ( AI_SUCCESS == mtl->Get<aiColor4D>( AI_MATKEY_COLOR_DIFFUSE, dcolor ) ) {
			matSource.mMaterial.setDiffuse( ai::get( dcolor ) );
		}
		if ( AI_SUCCESS == mtl->Get<aiColor4D>( AI_MATKEY_COLOR_SPECULAR, scolor ) ) {
			matSource.mMaterial.setSpecular( ai::get( scolor ) );
		}
		if ( AI_SUCCESS == mtl->Get<aiColor4D>( AI_MATKEY_COLOR_AMBIENT, acolor ) ) {
			matSource.mMaterial.setAmbient( ai::get( acolor ) );
		}
		if ( AI_SUCCESS == mtl->Get<aiColor4D>( AI_MATKEY_COLOR_EMISSIVE, ecolor ) ) {
			matSource.mMaterial.setEmission( ai::get( ecolor ) );
		}
//		if ( AI_SUCCESS == mtl->Get<aiColor4D>( AI_MATKEY_COLOR_TRANSPARENT, tcolor ) ) {
//			matSource.mTransparentColor =  ai::get( tcolor );
//		}
//		if ( AI_SUCCESS == mtl->Get<float>( AI_MATKEY_OPACITY, opacity ) ) {
//			matSource.mUseAlpha =  (opacity < 1.0f - ci::EPSILON );
//		}
		
		
		for( auto& textureType : model::MaterialSource::sTextureTypes ) {
			aiString textureFileName;
			// FIXME: We're only loading the first texture of each type (index 0)
			if ( AI_SUCCESS == mtl->GetTexture( static_cast<aiTextureType>(textureType), 0, &textureFileName ) ) {
				ci::fs::path texturePath;
				if( rootPath.empty() ) {
					texturePath = modelPath.parent_path() / ci::fs::path( textureFileName.data );
				} else {
					texturePath = rootPath / ci::fs::path( textureFileName.data );
				}
				LOG_M << " [" << texturePath.string() << "] of texture type: " << textureType << std::endl;
				
				if( boost::filesystem::exists( texturePath ) ) {
					loadSurface( texturePath, mtl, textureType, surfacePool, &matSource );
				}
			}
		}

		return matSource;
	}
	
	std::vector<model::Weights> getBoneWeights( const aiMesh* aimesh, const std::unordered_map<std::string, std::shared_ptr<Node>>& bones )
	{
		// Create a list of empty bone weights mirroring the # of vertices
		std::vector<model::Weights> weights( aimesh->mNumVertices, model::Weights() );

		for( unsigned b=0; b < aimesh->mNumBones; ++b ){
			model::NodeRef bone = bones.at( ai::get( aimesh->mBones[b]->mName ) );
			
			// Set the bone offset matrix if it hasn't been already
			if( bone->getOffset() == nullptr ) {
				bone->setOffsetMatrix( ai::get( aimesh->mBones[b]->mOffsetMatrix ) ) ;
			}
			
			// Add the bone weight information to the correct vertex index
			aiBone* aibone = aimesh->mBones[b];
			for( unsigned int w=0; w<aibone->mNumWeights; ++w ) {
				float weight = aibone->mWeights[w].mWeight;
				size_t index = size_t( aibone->mWeights[w].mVertexId );
				weights.at( index ).addWeight( bone, weight );
			}
		}
		return weights;
	}
	
	const aiNode* findMeshNode( const std::string& meshName, const aiScene* aiscene, const aiNode* ainode )
	{
		for( unsigned i=0; i<ainode->mNumMeshes; ++i ) {
			if( meshName == ai::get( aiscene->mMeshes[ ainode->mMeshes[i] ]->mName ) ) {
				return ainode;
			}
		}
		
		for( unsigned n=0; n < ainode->mNumChildren; ++n) {
			const aiNode* goalNode = findMeshNode( meshName, aiscene, ainode->mChildren[n] );
			if( goalNode != nullptr )
				return goalNode;
		}
		
		return nullptr;
	}
	
	model::NodeRef generateNodeHierarchy( std::unordered_map<std::string, std::shared_ptr<Node>>* bones,
										 const aiNode* ainode,
										 const std::unordered_set<std::string>& boneNames,
										 const std::shared_ptr<model::Node>& parent = nullptr,
										 mat4 derivedTransformation = mat4{},
										 int level = 0 )
	{
		assert( ainode );
		
		derivedTransformation *= ai::get( ainode->mTransformation );
		std::string name = ai::get( ainode->mName );
		
		// store transform
		aiVector3D position, scaling;
		aiQuaternion rotation;
		ainode->mTransformation.Decompose( scaling, rotation, position );
		model::NodeRef node = model::Node::create( ai::get(position), ai::get(rotation), ai::get(scaling), name, parent, level );
		
		if( boneNames.count(name) > 0 && bones->count( name ) == 0 ) {
			node->setBoneIndex( bones->size() );
			bones->emplace( name, node );
		}
		
		for( unsigned int c=0; c < ainode->mNumChildren; ++c) {
			model::NodeRef child = generateNodeHierarchy( bones, ainode->mChildren[c], boneNames, node, derivedTransformation, level + 1);
			node->addChild( child );
		}
		return node;
	}
	
	
	void generateAnimationCurves( const std::unordered_map<std::string, std::shared_ptr<Node>>& bones, const aiScene* aiscene )
	{
		for( unsigned int a=0; a < aiscene->mNumAnimations; ++a) {
			aiAnimation* anim = aiscene->mAnimations[a];
			
			for( unsigned int c=0; c < anim->mNumChannels; ++c ) {
				aiNodeAnim* nodeAnim = anim->mChannels[c];
				model::NodeRef bone;
				auto name = ai::get( nodeAnim->mNodeName );
				if( bones.count( name ) ) {
					bone = bones.at( name );
				}
				if( bone ) {
					float tsecs = ( anim->mTicksPerSecond != 0 ) ? (float) anim->mTicksPerSecond : 25.0f;
					bone->addAnimTrack( a, float( anim->mDuration ), tsecs );
					LOG_M << " Duration: " << anim->mDuration << " seconds:" << tsecs << std::endl;
					for( unsigned int k=0; k < nodeAnim->mNumPositionKeys; ++k) {
						const aiVectorKey& key = nodeAnim->mPositionKeys[k];
						bone->addPositionKeyframe( a, (float) key.mTime, ai::get( key.mValue ) );
					}
					for( unsigned int k=0; k < nodeAnim->mNumRotationKeys; ++k) {
						const aiQuatKey& key = nodeAnim->mRotationKeys[k];
						bone->addRotationKeyframe( a, (float) key.mTime, ai::get( key.mValue ) );
					}
					for( unsigned int k=0; k < nodeAnim->mNumScalingKeys; ++k) {
						const aiVectorKey& key = nodeAnim->mScalingKeys[k];
						bone->addScalingKeyframe( a, (float) key.mTime, ai::get( key.mValue ) );
					}
				} else {
					LOG_M << "Anim node " << ai::get(nodeAnim->mNodeName) << " is not a bone." << std::endl;
				}
			}
		}
	}
	
	std::vector<AnimInfo> getAnimInfos( const aiScene* aiscene )
	{
		std::vector<AnimInfo> mAnimInfos;
		for( unsigned int a=0; a < aiscene->mNumAnimations; ++a) {
			aiAnimation* anim = aiscene->mAnimations[a];
			
			mAnimInfos.push_back( AnimInfo{ (float) anim->mDuration, (float) anim->mTicksPerSecond, ai::get( anim->mName ) } );
		}
		
		return mAnimInfos;
	}
	
	std::unordered_set<std::string>  getBoneNames( const aiScene* aiscene )
	{
		std::unordered_set<std::string> boneNames;
		for( unsigned int m=0; m < aiscene->mNumMeshes; ++m ) {
			aiMesh * mesh = aiscene->mMeshes[m];
			for( unsigned int b=0; b < mesh->mNumBones; ++b) {
				std::string name = ai::get( mesh->mBones[b]->mName );
				boneNames.insert( name );
			}
		}
		return boneNames;
	}
	
} //end namespace ai

AssimpLoader::AssimpLoader( const ci::DataSourceRef& dataSource, const Settings& settings )
: mRootAssetFolderPath( settings.mRootAssetFolderPath )
, mSurfacePool( settings.mSurfacePool )
{
	// Assimp importer instance which cannot be destroyed until the scene loading is complete.
	std::unique_ptr<Assimp::Importer> importer( new Assimp::Importer() );
	importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
//	importer->SetIOHandler( new CustomIOSystem() );
	
	loadScene( loadAiScene( dataSource, importer.get(), settings.mFlags ) );
	for( auto& target : settings.mMorphTargets ) {
		loadMorphTarget( loadAiScene( target, importer.get(), settings.mFlags ) );
	}
}


const aiScene * AssimpLoader::loadAiScene( const ci::DataSourceRef& dataSource, Assimp::Importer* importer, unsigned int flags )
{
	const aiScene* aiscene = nullptr;
	if( dataSource->isFilePath() ) {
		mModelPath = dataSource->getFilePath();

		if( !importer->IsExtensionSupported( mModelPath.extension().string() ) )
			throw LoadErrorException( "Extension not supported." );

		aiscene = importer->ReadFile( mModelPath.string(), flags );
	}
	else {
		throw LoadErrorException( "Only loadAsset() is supported at the moment." );
	}
	if( !aiscene )
		throw LoadErrorException( importer->GetErrorString() );

	return aiscene;
}

void AssimpLoader::loadScene( const aiScene* aiscene )
{
	if( !aiscene->HasMeshes() )
		LOG_E << "Scene has no meshes.";
	
	if( !mSurfacePool ) {
		mSurfacePool = SurfacePoolRef( new SurfacePool );
	}
	
	const auto& boneNames = ai::getBoneNames( aiscene );
	
	const aiNode* root = aiscene->mRootNode;
	
	if( !boneNames.empty() ) {
		mRootNode = ai::generateNodeHierarchy( &mBones, root, boneNames );
		if( aiscene->HasAnimations() ) {
			ai::generateAnimationCurves( mBones, aiscene );
		}
	}
	
	for( unsigned int m=0; m < aiscene->mNumMeshes; ++m ) {
		aiMesh * mesh = aiscene->mMeshes[m];
		std::string name = ai::get( mesh->mName );
		
		SectionSourceRef section = std::make_shared<SectionSource>();
		section->mName			= ai::get( mesh->mName );
		section->mIndices		= ai::getIndices( mesh );
		section->mPositions		= ai::getPositions( mesh );
		section->mNormals		= ai::getNormals( mesh );
		section->mTangents		= ai::getTangents( mesh );
		section->mBitangents	= ai::getBiTangents( mesh );
		section->mTexCoords		= ai::getTexCoords( mesh );
		section->mMaterialSource = ai::getMaterial( aiscene, mesh, mModelPath, mSurfacePool, mRootAssetFolderPath );
		if( mesh->HasBones() ) {
			section->mWeights = ai::getBoneWeights( mesh, mBones );
			for( const auto& boneWeight : section->mWeights ) {
				vec4 vWeights = vec4{};
				vec4 vIndices = vec4{};
				for( unsigned int b =0; b < boneWeight.getNumActiveWeights(); ++b ) {
					const Node* bone = boneWeight.getBone(b);
					vWeights[b] = boneWeight.getWeight(b);
					//FIXME: Maybe use ints on the desktop?
					vIndices[b] = static_cast<float>( bone->getBoneIndex() );
				}
				section->mBoneIndices.push_back( vIndices );
				section->mBoneWeights.push_back( vWeights );
			}
		} else {
			const aiNode* ainode = ai::findMeshNode( name, aiscene, aiscene->mRootNode );
			if( ainode ) {
				section->mDefaultTransformation = ai::get( ainode->mTransformation);
			}
		}
		
		mSectionSources.push_back( section );
	}
	
	if( aiscene->HasAnimations() ) {
		mAnimInfos = ai::getAnimInfos( aiscene );
	}
}

void AssimpLoader::loadMorphTarget( const aiScene* aiscene )
{
	assert( aiscene->HasMeshes() );
	assert( mSurfacePool );

	for( unsigned int m = 0; m < aiscene->mNumMeshes; ++m ) {
		aiMesh * mesh = aiscene->mMeshes[m];
		auto positions = ai::getPositions( mesh );

		auto& section = mSectionSources.at( m );
		assert( section->mPositions.size() == positions.size() );

		size_t i = 0;
		float diffSum = 0.0f;
		for( auto& p : positions ) {
			auto diff = p - section->mPositions.at( i++ );
			diffSum += length2( diff );
			p = diff;
		}

		if( diffSum > ci::EPSILON ) {
			section->mMorphOffsets.emplace_back( positions );
		}
	}

}

} //end namespace model
