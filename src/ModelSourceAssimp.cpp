#include "ModelSourceAssimp.h"
#include "CustomIOStream.h"
#include "Skeleton.h"
#include "Debug.h"

#include "assimp/postprocess.h"
#include "cinder/ImageIo.h"
#include "cinder/app/App.h"

#include <boost/algorithm/string.hpp>

namespace ai {
	unsigned int flags =
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
	aiProcess_FindInvalidData |
	aiProcess_OptimizeMeshes;
//	aiProcess_SplitLargeMeshes |
//	aiProcess_FindInstances |
	
	ci::Vec3f get( const aiVector3D &v )
	{
		return ci::Vec3f( v.x, v.y, v.z );
	}
	
	ci::Quatf get( const aiQuaternion &q )
	{
		return ci::Quatf( q.w, q.x, q.y, q.z );
	}
	
	ci::Matrix44f get( const aiMatrix4x4 &m )
	{
		return ci::Matrix44f( &m.a1, true );
	}
	
	ci::ColorAf get( const aiColor4D &c )
	{
		return ci::ColorAf( c.r, c.g, c.b, c.a );
	}
	
	std::string get( const aiString &s )
	{
		return std::string( s.data );
	}
	
	std::vector<ci::Vec3f> getPositions( const aiMesh* aimesh )
	{
		std::vector<ci::Vec3f> positions;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			positions.push_back( ai::get( aimesh->mVertices[i] ) );
		}
		return positions;
	}
	
	std::vector<ci::Vec3f> getNormals( const aiMesh* aimesh )
	{
		std::vector<ci::Vec3f> normals;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			normals.push_back( -ai::get( aimesh->mNormals[i] ) );
		}
		return normals;
	}
	
	std::vector<ci::Vec2f> getTexCoords( const aiMesh* aimesh )
	{
		std::vector<ci::Vec2f> texCoords;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			texCoords.push_back( ci::Vec2f(aimesh->mTextureCoords[0][i].x,
										   aimesh->mTextureCoords[0][i].y) );
		}
		assert( texCoords.size() > 0 );
		return texCoords;
	}
	
	std::vector<uint32_t> getIndices( const aiMesh* aimesh )
	{
		std::vector<uint32_t> indices;
		for( unsigned int i=0; i < aimesh->mNumFaces; ++i ) {
			aiFace aiface = aimesh->mFaces[i];
			unsigned numIndices = aiface.mNumIndices;
			assert( numIndices <= 3 );
			for(int n=0; n < 3; ++n) {
				if ( numIndices == 2 && n == 2 ) {
					indices.push_back( aiface.mIndices[1] );
				} else {
					indices.push_back( aiface.mIndices[n] );
				}
			}
		}
		return indices;
	}
	
	model::MaterialInfo getTexture( const aiScene* aiscene, const aiMesh *aimesh, ci::fs::path modelPath, ci::fs::path rootPath )
	{
		model::MaterialInfo matInfo;
		// Handle material info
		aiMaterial *mtl = aiscene->mMaterials[ aimesh->mMaterialIndex ];
		
		aiString name;
		mtl->Get( AI_MATKEY_NAME, name );
		LOG_M << "material " << ai::get( name ) << std::endl;
		// Culling
		int twoSided;
		if ( ( AI_SUCCESS == mtl->Get( AI_MATKEY_TWOSIDED, twoSided ) ) && twoSided ) {
			matInfo.mTwoSided = true;
			matInfo.mMaterial.setFace( GL_FRONT_AND_BACK );
			LOG_M << " two sided" << std::endl;
		} else {
			matInfo.mTwoSided = false;
			matInfo.mMaterial.setFace( GL_FRONT );
		}
		
		aiColor4D dcolor, scolor, acolor, ecolor, tcolor;
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_DIFFUSE, dcolor ) ) {
			matInfo.mMaterial.setDiffuse( ai::get( dcolor ) );
			LOG_M << " diffuse: " << ai::get( dcolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_SPECULAR, scolor ) ) {
			matInfo.mMaterial.setSpecular( ai::get( scolor ) );
			LOG_M << " specular: " << ai::get( scolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_AMBIENT, acolor ) ) {
			matInfo.mMaterial.setAmbient( ai::get( acolor ) );
			LOG_M << " ambient: " << ai::get( acolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_EMISSIVE, ecolor ) ) {
			matInfo.mMaterial.setEmission( ai::get( ecolor ) );
			LOG_M << " emission: " << ai::get( ecolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_TRANSPARENT, tcolor ) ) {
			matInfo.mTransparentColor =  ai::get( tcolor );
			LOG_M << " transparent: " << ai::get( tcolor ) << std::endl;
		}
		
		// Load Textures
		int texIndex = 0;
		aiString texPath;
		
		// TODO: handle other aiTextureTypes
		if ( AI_SUCCESS == mtl->GetTexture( aiTextureType_DIFFUSE, texIndex, &texPath ) ) {
			LOG_M << " diffuse texture " << texPath.data;
			ci::fs::path texFsPath( texPath.data );
			ci::fs::path modelFolder = modelPath.parent_path();
			ci::fs::path realPath;
			if( rootPath.empty() ) {
				realPath = modelFolder / texFsPath;
			} else {
				realPath = rootPath / texFsPath;
			}
			
			int texFlag;
			if ( AI_SUCCESS == mtl->Get( AI_MATKEY_TEXFLAGS(aiTextureType_DIFFUSE, 0), texFlag ) ) {
				if( texFlag == aiTextureFlags_UseAlpha ) {
					matInfo.mUseAlpha = true;
					LOG_M << " Texture uses alpha." << std::endl;
				}
			}
			
			LOG_M << " [" << realPath.string() << "]" << std::endl;
			
			// texture wrap
			ci::gl::Texture::Format format;
			int uwrap;
			if ( AI_SUCCESS == mtl->Get( AI_MATKEY_MAPPINGMODE_U_DIFFUSE( 0 ), uwrap ) ) {
				switch ( uwrap ) {
					case aiTextureMapMode_Wrap:
						format.setWrapS( GL_REPEAT );
						break;
						
					case aiTextureMapMode_Clamp:
						format.setWrapS( GL_CLAMP );
						break;
						
					case aiTextureMapMode_Decal:
						// If the texture coordinates for a pixel are outside [0...1]
						// the texture is not applied to that pixel.
						format.setWrapS( GL_CLAMP_TO_EDGE );
						break;
						
					case aiTextureMapMode_Mirror:
						// A texture coordinate u|v becomes u%1|v%1 if (u-(u%1))%2
						// is zero and 1-(u%1)|1-(v%1) otherwise.
						// TODO
						format.setWrapS( GL_REPEAT );
						break;
				}
			}
			int vwrap;
			if ( AI_SUCCESS == mtl->Get( AI_MATKEY_MAPPINGMODE_V_DIFFUSE( 0 ), vwrap ) ) {
				switch ( vwrap ) {
					case aiTextureMapMode_Wrap:
						format.setWrapT( GL_REPEAT );
						break;
						
					case aiTextureMapMode_Clamp:
						format.setWrapT( GL_CLAMP );
						break;
						
					case aiTextureMapMode_Decal:
						// If the texture coordinates for a pixel are outside [0...1]
						// the texture is not applied to that pixel.
						format.setWrapT( GL_CLAMP_TO_EDGE );
						break;
						
					case aiTextureMapMode_Mirror:
						// A texture coordinate u|v becomes u%1|v%1 if (u-(u%1))%2
						// is zero and 1-(u%1)|1-(v%1) otherwise.
						// TODO
						format.setWrapT( GL_REPEAT );
						break;
				}
			}
			
			std::string ext = realPath.extension().string();
			boost::algorithm::to_lower( ext );
			if ( ext == ".dds" ) {
				// FIXME: loadDds does not seem to work with mipmaps in the latest cinder version
				// fix based on the work of javi.agenjo, https://github.com/gaborpapp/Cinder/commit/3e7302
				matInfo.mTexture = ci::gl::Texture::loadDds( ci::loadFile( realPath )->createStream(), format );
				if ( !matInfo.mTexture )
					LOG_M << "failed to laod dds..." << std::endl;
			} else {
				matInfo.mTexture = ci::gl::Texture( ci::loadImage( realPath ), format );
			}
		}
		return matInfo;
	}
	
	std::vector<model::BoneWeights> getBoneWeights( const aiMesh* aimesh, const model::Skeleton* skeleton )
	{
		std::vector<model::BoneWeights> boneWeights;
		unsigned int nbBones = aimesh->mNumBones;
		std::string name = ai::get( aimesh->mName );
		
		// Create a list of empty bone weights mirroring the # of vertices
		for( unsigned v=0; v < aimesh->mNumVertices; ++v ) {
			boneWeights.push_back( model::BoneWeights() );
		}
		
		for( unsigned b=0; b < nbBones; ++b ){
			model::NodeRef bone = skeleton->getBone( ai::get( aimesh->mBones[b]->mName ) );
			
			// Set the bone offset matrix if it hasn't been already
			if( bone->getOffset() == nullptr ) {
				bone->setOffsetMatrix( ai::get( aimesh->mBones[b]->mOffsetMatrix ) ) ;
			}
			
			// Add the bone weight information to the correct vertex index
			aiBone* aibone = aimesh->mBones[b];
			for( unsigned int w=0; w<aibone->mNumWeights; ++w ) {
				float weight = aibone->mWeights[w].mWeight;
				int boneWeightIndex = int( aibone->mWeights[w].mVertexId );
				boneWeights[boneWeightIndex].addWeight( bone, weight );
			}
		}
		return boneWeights;
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
	
	model::NodeRef generateNodeHierarchy(model::Skeleton* skeleton,
										 const aiNode* ainode,
										 const std::unordered_set<std::string>& boneNames,
										 const std::shared_ptr<model::Node>& parent = nullptr,
										 ci::Matrix44f derivedTransformation = ci::Matrix44f::identity(),
										 int level = 0 )
	{
		assert( ainode );
		
		derivedTransformation *= ai::get( ainode->mTransformation );
		std::string name = ai::get( ainode->mName );
		
		// store transform
		aiVector3D position, scaling;
		aiQuaternion rotation;
		ainode->mTransformation.Decompose( scaling, rotation, position );
		model::NodeRef node = model::NodeRef( new model::Node( ai::get(position), ai::get(rotation), ai::get(scaling), name, parent, level ) );
		
		if( boneNames.count(name) > 0 ) {
			skeleton->addBone(name, node);
		}
		
		for( unsigned int c=0; c < ainode->mNumChildren; ++c) {
			model::NodeRef child = generateNodeHierarchy( skeleton, ainode->mChildren[c], boneNames, node, derivedTransformation, level + 1);
			node->addChild( child );
		}
		return node;
	}
	
	
	void generateAnimationCurves( model::SkeletonRef skeleton, const aiScene* aiscene )
	{
		for( unsigned int a=0; a < aiscene->mNumAnimations; ++a) {
			aiAnimation* anim = aiscene->mAnimations[a];
			
			skeleton->setAnimInfo( int(a), (float) anim->mDuration, (float) anim->mTicksPerSecond, ai::get( anim->mName ) );
			
			for( unsigned int c=0; c < anim->mNumChannels; ++c ) {
				aiNodeAnim* nodeAnim = anim->mChannels[c];
				model::NodeRef bone = skeleton->getBone( ai::get(nodeAnim->mNodeName) );
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
	
	model::SkeletonRef getSkeleton( const aiScene* aiscene, bool hasAnimations, const aiNode* root )
	{
		root = ( root ) ? root : aiscene->mRootNode;
		
		std::unordered_set<std::string> boneNames;
		for( unsigned int m=0; m < aiscene->mNumMeshes; ++m ) {
			aiMesh * mesh = aiscene->mMeshes[m];
			for( unsigned int b=0; b < mesh->mNumBones; ++b) {
				std::string name = ai::get( mesh->mBones[b]->mName );
				boneNames.insert( name );
			}
		}
		
		model::SkeletonRef skeleton = model::Skeleton::create();
		skeleton->setRootNode( generateNodeHierarchy( skeleton.get(), root, boneNames ) );
		if( hasAnimations ) {
			generateAnimationCurves( skeleton, aiscene );
		}
		return skeleton;
	}
	
} //end namespace ai

namespace model {
	
	void ModelSourceAssimp::SectionInfo::log()
	{
		LOG_V	<< "Normals :" << mHasNormals
				<< " Skeleton :" << mHasSkeleton
				<< " Materials :" << mHasMaterials << std::endl;
	}

ModelSourceAssimp::ModelSourceAssimp( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath )
{
	mModelPath = modelPath;
	mRootAssetFolderPath = rootAssetFolderPath;
	
	mImporter = std::unique_ptr<Assimp::Importer>( new Assimp::Importer() );
	mImporter->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
//	mImporter->SetIOHandler( new CustomIOSystem() );
	mAiScene = mImporter->ReadFile( mModelPath.string(), ai::flags );
	
	if( !mAiScene ) {
		LOG_M << mImporter->GetErrorString() << std::endl;
		throw LoadErrorException( mImporter->GetErrorString() );
	} else if ( !mAiScene->HasMeshes() ) {
		throw LoadErrorException( "Scene has no meshes." );
	}
	
	for( unsigned int m=0; m < mAiScene->mNumMeshes; ++m ) {
		aiMesh * mesh = mAiScene->mMeshes[m];
		SectionInfo sectionInfo;
		sectionInfo.mHasSkeleton = mesh->HasBones();
		sectionInfo.mHasNormals = mesh->HasNormals();
		sectionInfo.mNumVertices = mesh->mNumVertices;
		sectionInfo.mNumIndices = 3 * mesh->mNumFaces;
		sectionInfo.mHasMaterials = mAiScene->HasMaterials() && mesh->GetNumUVChannels() > 0;
		mSections.push_back( sectionInfo );
	}

	mHasAnimations = mAiScene->HasAnimations();
	for( const SectionInfo& section : mSections ) {
		mHasSkeleton = mHasSkeleton || section.mHasSkeleton;
	}
}

ModelSourceAssimpRef ModelSourceAssimp::create( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath )
{
	return ModelSourceAssimpRef( new ModelSourceAssimp( modelPath, rootAssetFolderPath ) );
}

void ModelSourceAssimp::load( ModelTarget *target )
{
	SkeletonRef skeleton = target->getSkeleton();
	if( mHasSkeleton && skeleton == nullptr ) {
		skeleton = ai::getSkeleton( mAiScene, mHasAnimations );
	}
	
	for( unsigned int i=0; i< mAiScene->mNumMeshes; ++i ) {
		mSections[i].log();

		const aiMesh* aimesh = mAiScene->mMeshes[i];
		std::string name = ai::get( aimesh->mName );
		
		LOG_M	<< "loading mesh " << i << " [" << name << "]"
				<< " #faces:" << aimesh->mNumFaces
				<< " #vertices:" << aimesh->mNumVertices << std::endl;
		
		target->setActiveSection( i );
		target->loadName( name );
		target->loadIndices( ai::getIndices( aimesh ) );
		target->loadVertexPositions( ai::getPositions( aimesh ) );
		
		if( mSections[i].mHasNormals ) {
			target->loadVertexNormals( ai::getNormals( aimesh ) );
		}
		
		if( mSections[i].mHasMaterials ) {
			target->loadTex( ai::getTexCoords( aimesh ),
							 ai::getTexture(mAiScene, aimesh, mModelPath, mRootAssetFolderPath ) );
		}
		
		if( mSections[i].mHasSkeleton && skeleton ) {
			target->loadSkeleton( skeleton );
			target->loadBoneWeights( ai::getBoneWeights( aimesh, skeleton.get() ) );
		} else {
			const aiNode* ainode = ai::findMeshNode( name, mAiScene, mAiScene->mRootNode );
			if( ainode ) {
				target->loadDefaultTransformation( ai::get( ainode->mTransformation) );
			}
		}
	}
}

} //end namespace model
