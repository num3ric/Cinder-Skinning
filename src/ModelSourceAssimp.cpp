#include "ModelSourceAssimp.h"
#include "CustomIOStream.h"
#include "Skeleton.h"

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
	aiProcess_Triangulate |
	aiProcess_GenUVCoords |
	aiProcess_SortByPType |
	aiProcess_FindDegenerates |
	aiProcess_FindInvalidData |
	aiProcess_OptimizeMeshes;
//	aiProcess_SplitLargeMeshes |
//	aiProcess_FindInstances |
	
	
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
	
	model::SkeletonRef loadSkeleton( bool hasAnimations, const aiScene* aiscene, const aiNode* root )
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
		
		model::SkeletonRef skeleton = model::Skeleton::create( boneNames );
		skeleton->setRootNode( generateNodeHierarchy( skeleton.get(), root ) );
		if( hasAnimations ) {
			generateAnimationCurves( skeleton.get(), aiscene );
		}
		return skeleton;
	}
	
	model::NodeRef generateNodeHierarchy( model::Skeleton* skeleton, const aiNode* ainode, const model::NodeRef& parent, ci::Matrix44f derivedTransformation, int level )
	{
		if( !ainode )
			throw "Invalid ainode";
		
		derivedTransformation *= ai::get( ainode->mTransformation );
		std::string name = ai::get( ainode->mName );
		
		model::NodeRef node = model::NodeRef( new model::Node( derivedTransformation, ai::get( ainode->mTransformation ), name, parent, level ) );
		
		if( skeleton->hasBone( name ) ) {
			node->setBoneIndex( skeleton->findBoneIndex( name ) );
			skeleton->insertBone(name, node);
		}
		
		for( unsigned int c=0; c < ainode->mNumChildren; ++c) {
			model::NodeRef child = generateNodeHierarchy( skeleton, ainode->mChildren[c], node, derivedTransformation, level + 1);
			node->addChild( child );
		}
		return node;
	}
	
	
	void generateAnimationCurves( model::Skeleton* skeleton, const aiScene* aiscene )
	{
		//TODO: Handle multiple animations correctly, right now this actually works only for one aiAnimation
		for( unsigned int a=0; a < aiscene->mNumAnimations; ++a) {
			aiAnimation* anim = aiscene->mAnimations[a];
			
			skeleton->mAnimationDuration = 2.0f;
			
			for( unsigned int c=0; c < anim->mNumChannels; ++c ) {
				aiNodeAnim* nodeAnim = anim->mChannels[c];
				
				try {
					model::NodeRef bone = skeleton->getBone( ai::get(nodeAnim->mNodeName) );
					float tsecs = ( anim->mTicksPerSecond != 0 ) ? (float) anim->mTicksPerSecond : 25.0f;
					bone->initAnimation( float( anim->mDuration ), tsecs );
					ci::app::console() << " Duration: " << anim->mDuration << " seconds:" << tsecs << std::endl;
					for( unsigned int k=0; k < nodeAnim->mNumPositionKeys; ++k) {
						const aiVectorKey& key = nodeAnim->mPositionKeys[k];
						bone->addTranslationKeyframe( (float) key.mTime, ai::get( key.mValue ) );
					}
					for( unsigned int k=0; k < nodeAnim->mNumRotationKeys; ++k) {
						const aiQuatKey& key = nodeAnim->mRotationKeys[k];
						bone->addRotationKeyframe( (float) key.mTime, ai::get( key.mValue ) );
					}
					for( unsigned int k=0; k < nodeAnim->mNumScalingKeys; ++k) {
						const aiVectorKey& key = nodeAnim->mScalingKeys[k];
						bone->addScalingKeyframe( (float) key.mTime, ai::get( key.mValue ) );
					}
				} catch ( const std::out_of_range& ) {
					ci::app::console() << "Anim node " << ai::get(nodeAnim->mNodeName) << " is not a bone." << std::endl;
				}
			}
		}
	}
	
	void loadPositions( const aiMesh* aimesh, std::vector<ci::Vec3f>* positions )
	{
		std::vector<ci::Vec3f> vertices;
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			positions->push_back( ai::get( aimesh->mVertices[i] ) );
		}
	}
	
	void loadNormals( const aiMesh* aimesh, std::vector<ci::Vec3f>* normals )
	{
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			normals->push_back( -ai::get( aimesh->mNormals[i] ) );
		}
	}
	
	
	
	void loadTexCoords( const aiMesh* aimesh, std::vector<ci::Vec2f>* texCoords )
	{
		for( unsigned int i=0; i < aimesh->mNumVertices; ++i ) {
			texCoords->push_back( ci::Vec2f(aimesh->mTextureCoords[0][i].x,
									   aimesh->mTextureCoords[0][i].y) );
		}
	}
	
	void loadIndices( const aiMesh* aimesh, std::vector<uint32_t>* indices )
	{
		for( unsigned int i=0; i < aimesh->mNumFaces; ++i ) {
			aiFace aiface = aimesh->mFaces[i];
			unsigned numIndices = aiface.mNumIndices;
			assert( numIndices <= 3 );
			for(int n=0; n < 3; ++n) {
				if ( numIndices == 2 && n == 2 ) {
					indices->push_back( aiface.mIndices[1] );
				} else {
					indices->push_back( aiface.mIndices[n] );
				}
			}
		}
	}
	
	void loadTexture( const aiScene* aiscene, const aiMesh *aimesh, model::MaterialInfo* matInfo, ci::fs::path modelPath, ci::fs::path rootPath )
	{
		// Handle material info
		aiMaterial *mtl = aiscene->mMaterials[ aimesh->mMaterialIndex ];
		
		aiString name;
		mtl->Get( AI_MATKEY_NAME, name );
		ci::app::console() << "material " << ai::get( name ) << std::endl;
		// Culling
		int twoSided;
		if ( ( AI_SUCCESS == mtl->Get( AI_MATKEY_TWOSIDED, twoSided ) ) && twoSided ) {
			matInfo->mTwoSided = true;
			matInfo->mMaterial.setFace( GL_FRONT_AND_BACK );
			ci::app::console() << " two sided" << std::endl;
		} else {
			matInfo->mTwoSided = false;
			matInfo->mMaterial.setFace( GL_FRONT );
		}
		
		aiColor4D dcolor, scolor, acolor, ecolor, tcolor;
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_DIFFUSE, dcolor ) ) {
			matInfo->mMaterial.setDiffuse( ai::get( dcolor ) );
			ci::app::console() << " diffuse: " << ai::get( dcolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_SPECULAR, scolor ) ) {
			matInfo->mMaterial.setSpecular( ai::get( scolor ) );
			ci::app::console() << " specular: " << ai::get( scolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_AMBIENT, acolor ) ) {
			matInfo->mMaterial.setAmbient( ai::get( acolor ) );
			ci::app::console() << " ambient: " << ai::get( acolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_EMISSIVE, ecolor ) ) {
			matInfo->mMaterial.setEmission( ai::get( ecolor ) );
			ci::app::console() << " emission: " << ai::get( ecolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_TRANSPARENT, tcolor ) ) {
			matInfo->mTransparentColor =  ai::get( tcolor );
			ci::app::console() << " transparent: " << ai::get( tcolor ) << std::endl;
		}
		
		// Load Textures
		int texIndex = 0;
		aiString texPath;
		
		// TODO: handle other aiTextureTypes
		if ( AI_SUCCESS == mtl->GetTexture( aiTextureType_DIFFUSE, texIndex, &texPath ) ) {
			ci::app::console() << " diffuse texture " << texPath.data;
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
					matInfo->mUseAlpha = true;
					ci::app::console() << " Texture uses alpha." << std::endl;
				}
			}
			
			ci::app::console() << " [" << realPath.string() << "]" << std::endl;
			
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
				matInfo->mTexture = ci::gl::Texture::loadDds( ci::loadFile( realPath )->createStream(), format );
				if ( !matInfo->mTexture )
					ci::app::console() << "failed to laod dds..." << std::endl;
			} else {
				matInfo->mTexture = ci::gl::Texture( ci::loadImage( realPath ), format );
			}
		}
	}
	
	void loadBoneWeights( const aiMesh* aimesh, const model::Skeleton* skeleton, std::vector<model::BoneWeights>* boneWeights  )
	{
		unsigned int nbBones = aimesh->mNumBones;
		std::string name = ai::get( aimesh->mName );
		
		// Create a list of empty bone weights mirroring the # of vertices
		for( unsigned v=0; v < aimesh->mNumVertices; ++v ) {
			boneWeights->push_back( model::BoneWeights() );
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
				(*boneWeights)[boneWeightIndex].addWeight( bone, weight );
			}
		}
	}
}

namespace model {

ModelSourceAssimp::ModelSourceAssimp( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath )
{
	mModelPath = modelPath;
	mRootAssetFolderPath = rootAssetFolderPath;
	
	mImporter = std::unique_ptr<Assimp::Importer>( new Assimp::Importer() );
	mImporter->SetIOHandler( new CustomIOSystem() );
	mAiScene = mImporter->ReadFile( mModelPath.string(), ai::flags );
	
	//TODO: make own exception class to catch
	if( !mAiScene ) {
		ci::app::console() << mImporter->GetErrorString() << std::endl;
		throw mImporter->GetErrorString();
	} else if ( !mAiScene->HasMeshes() ) {
		throw "Scene has no meshes.";
	}
	
	unsigned int numBones = 0;
	for( unsigned int m=0; m < mAiScene->mNumMeshes; ++m ) {
		aiMesh * mesh = mAiScene->mMeshes[m];
		numBones += mesh->mNumBones;
		mModelInfo.mHasNormals = mModelInfo.mHasNormals || mesh->HasNormals();
		
		mModelInfo.mNumVertices.push_back( mesh->mNumVertices );
		mModelInfo.mNumIndices.push_back( 3 * mesh->mNumFaces );
	}
	mModelInfo.mHasSkeleton = (numBones > 0);
	mModelInfo.mHasAnimations = mAiScene->HasAnimations();
	//TODO: old loader checked (aimesh->GetNumUVChannels() > 0), replace with it?
	mModelInfo.mHasMaterials = mAiScene->HasMaterials();
	mModelInfo.mNumSections = mAiScene->mNumMeshes;
}

ModelSourceAssimpRef ModelSourceAssimp::create( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath )
{
	return ModelSourceAssimpRef( new ModelSourceAssimp( modelPath, rootAssetFolderPath ) );
}

size_t getTotal( const std::vector<size_t> v )
{
	size_t total = 0;
	for( size_t i=0; i< v.size(); ++i ) {
		total += v[i];
	}
	return total;
}

size_t ModelSourceAssimp::getNumVertices( int section ) const
{
	if( section == -1 ) {
		return getTotal( mModelInfo.mNumVertices );
	}
	
	return mModelInfo.mNumVertices[section];
}
size_t ModelSourceAssimp::getNumIndices( int section )  const
{
	if( section == -1 ) {
		return getTotal( mModelInfo.mNumIndices );
	}

	return mModelInfo.mNumIndices[section];
}

void ModelSourceAssimp::load( ModelTarget *target )
{
	SkeletonRef skeleton = target->getSkeleton();
	bool loadSkeleton = false;
	if( mModelInfo.mHasSkeleton && skeleton == nullptr ) {
		skeleton = ai::loadSkeleton( mModelInfo.mHasAnimations, mAiScene );
		loadSkeleton = true;
	}
	
	for( unsigned int i=0; i< mAiScene->mNumMeshes; ++i ) {
		const aiMesh* aimesh = mAiScene->mMeshes[i];
		std::string name = ai::get( aimesh->mName );
		
		ci::app::console() << "loading mesh " << i;
		if ( name != "" )
			ci::app::console() << " [" << name << "]";
		ci::app::console() << " #faces: " << aimesh->mNumFaces;
		ci::app::console() << " #vertices: " << aimesh->mNumVertices << std::endl;
		
		target->setActiveSection( i );
		
		std::vector<ci::Vec3f> positions;
		std::vector<uint32_t> indices;
		ai::loadPositions( aimesh, &positions );
		ai::loadIndices( aimesh, &indices );
		target->loadName( name );
		target->loadIndices( indices );
		target->loadVertexPositions( positions );
		
		if( mModelInfo.mHasNormals ) {
			std::vector<ci::Vec3f> normals;
			ai::loadNormals( aimesh, &normals );
			target->loadVertexNormals( normals );
		}
		
		if( mModelInfo.mHasMaterials && aimesh->GetNumUVChannels() > 0 ) {
			std::vector<ci::Vec2f> texCoords;
			MaterialInfo matInfo;
			ai::loadTexCoords( aimesh, &texCoords );
			ai::loadTexture(mAiScene, aimesh, &matInfo, mModelPath, mRootAssetFolderPath );
			target->loadTex( texCoords, matInfo );
		}
		
//		if( mModelInfo.mHasSkeleton ) {
		if( aimesh->HasBones() ) {
			if( loadSkeleton )
				target->loadSkeleton( skeleton );
			
			std::vector<BoneWeights> boneWeights;
			ai::loadBoneWeights( aimesh, skeleton.get(), &boneWeights );
			target->loadBoneWeights( boneWeights );
		} else {
			const aiNode* ainode = ai::findMeshNode( name, mAiScene, mAiScene->mRootNode );
			if( ainode ) {
				target->loadDefaultTransformation( ai::get( ainode->mTransformation) );
			}
		}
//		}
	}
}

} //end namespace model
