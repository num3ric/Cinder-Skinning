#include "ModelSourceAssimp.h"
#include "CustomIOStream.h"
#include "Skeleton.h"

#include "assimp/postprocess.h"
#include "cinder/ImageIo.h"
#include "cinder/app/App.h"

#include <boost/algorithm/string.hpp>

using namespace ci;

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
		
		for( unsigned n=0; ainode->mNumChildren; ++n) {
			return findMeshNode( meshName, aiscene, ainode->mChildren[n] );
		}
		
		return nullptr;
	}
	
	SkeletonRef loadSkeleton( bool hasAnimations, const aiScene* aiscene, const aiNode* root )
	{
		SkeletonRef skeleton = Skeleton::create();
		
		root = ( root ) ? root : aiscene->mRootNode;
		
		for( unsigned int m=0; m < aiscene->mNumMeshes; ++m ) {
			aiMesh * mesh = aiscene->mMeshes[m];
			for( unsigned int b=0; b < mesh->mNumBones; ++b) {
				std::string name = ai::get( mesh->mBones[b]->mName );
				// We avoid duplicates since names are inserted within a map
				skeleton->insertBone(name, nullptr);
			}
		}
		
		skeleton->setRootNode( generateNodeHierarchy( skeleton.get(), root ) );
		if( hasAnimations ) {
			generateAnimationCurves( skeleton.get(), aiscene );
		}
		return skeleton;
	}
	
	NodeRef generateNodeHierarchy( Skeleton* skeleton, const aiNode* ainode, const NodeRef& parent, Matrix44f derivedTransformation, int level )
	{
		if( !ainode )
			throw "Invalid ainode";
		
		derivedTransformation *= ai::get( ainode->mTransformation );
		std::string name = ai::get( ainode->mName );
		
		NodeRef node = NodeRef( new Node( derivedTransformation, ai::get( ainode->mTransformation ), name, parent, level ) );
		
		if( skeleton->hasBone( name ) ) {
			int index = skeleton->getBoneIndex( name );
			node->setBoneIndex( index );
			skeleton->insertBone(name, node);
		}
		
		for( unsigned int c=0; c < ainode->mNumChildren; ++c) {
			NodeRef child = generateNodeHierarchy( skeleton, ainode->mChildren[c], node, derivedTransformation, level + 1);
			node->addChild( child );
		}
		return node;
	}
	
	
	void generateAnimationCurves( Skeleton* skeleton, const aiScene* aiscene )
	{
		//TODO: Handle multiple animations correctly, right now this actually works only for one aiAnimation
		for( unsigned int a=0; a < aiscene->mNumAnimations; ++a) {
			aiAnimation* anim = aiscene->mAnimations[a];
			
			skeleton->mAnimationDuration = 2.0f;
			
			for( unsigned int c=0; c < anim->mNumChannels; ++c ) {
				aiNodeAnim* nodeAnim = anim->mChannels[c];
				
				try {
					NodeRef bone = skeleton->getBone( ai::get(nodeAnim->mNodeName) );
					float tsecs = ( anim->mTicksPerSecond != 0 ) ? (float) anim->mTicksPerSecond : 25.0f;
					bone->initAnimation( anim->mDuration, tsecs );
					app::console() << " Duration: " << anim->mDuration << " seconds:" << tsecs << std::endl;
					for( int k=0; k < nodeAnim->mNumPositionKeys; ++k) {
						const aiVectorKey& key = nodeAnim->mPositionKeys[k];
						bone->addTranslationKeyframe( (float) key.mTime, ai::get( key.mValue ) );
					}
					for( int k=0; k < nodeAnim->mNumRotationKeys; ++k) {
						const aiQuatKey& key = nodeAnim->mRotationKeys[k];
						bone->addRotationKeyframe( (float) key.mTime, ai::get( key.mValue ) );
					}
					for( int k=0; k < nodeAnim->mNumScalingKeys; ++k) {
						const aiVectorKey& key = nodeAnim->mScalingKeys[k];
						bone->addScalingKeyframe( (float) key.mTime, ai::get( key.mValue ) );
					}
				} catch ( const std::out_of_range& oor) {
					app::console() << "Anim node " << ai::get(nodeAnim->mNodeName) << " is not a bone." << std::endl;
				}
			}
		}
	}
	
	void loadPositions( const aiMesh* aimesh, std::vector<Vec3f>* positions )
	{
		std::vector<Vec3f> vertices;
		for(int i=0; i < aimesh->mNumVertices; ++i) {
			positions->push_back( ai::get( aimesh->mVertices[i] ) );
		}
	}
	
	void loadNormals( const aiMesh* aimesh, std::vector<Vec3f>* normals )
	{
		for(int i=0; i < aimesh->mNumVertices; ++i) {
			normals->push_back( -ai::get( aimesh->mNormals[i] ) );
		}
	}
	
	
	
	void loadTexCoords( const aiMesh* aimesh, std::vector<Vec2f>* texCoords )
	{
		for(int i=0; i < aimesh->mNumVertices; ++i) {
			texCoords->push_back( Vec2f(aimesh->mTextureCoords[0][i].x,
									   aimesh->mTextureCoords[0][i].y) );
		}
	}
	
	void loadIndices( const aiMesh* aimesh, std::vector<uint32_t>* indices )
	{
		for(int i=0; i < aimesh->mNumFaces; ++i) {
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
	
	void loadTexture( const aiScene* aiscene, const aiMesh *aimesh, MaterialInfo* matInfo, fs::path modelPath, fs::path rootPath )
	{
		// Handle material info
		aiMaterial *mtl = aiscene->mMaterials[ aimesh->mMaterialIndex ];
		
		aiString name;
		mtl->Get( AI_MATKEY_NAME, name );
		app::console() << "material " << ai::get( name ) << std::endl;
		// Culling
		int twoSided;
		if ( ( AI_SUCCESS == mtl->Get( AI_MATKEY_TWOSIDED, twoSided ) ) && twoSided ) {
			matInfo->mTwoSided = true;
			matInfo->mMaterial.setFace( GL_FRONT_AND_BACK );
			app::console() << " two sided" << std::endl;
		} else {
			matInfo->mTwoSided = false;
			matInfo->mMaterial.setFace( GL_FRONT );
		}
		
		aiColor4D dcolor, scolor, acolor, ecolor, tcolor;
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_DIFFUSE, dcolor ) ) {
			matInfo->mMaterial.setDiffuse( ai::get( dcolor ) );
			app::console() << " diffuse: " << ai::get( dcolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_SPECULAR, scolor ) ) {
			matInfo->mMaterial.setSpecular( ai::get( scolor ) );
			app::console() << " specular: " << ai::get( scolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_AMBIENT, acolor ) ) {
			matInfo->mMaterial.setAmbient( ai::get( acolor ) );
			app::console() << " ambient: " << ai::get( acolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_EMISSIVE, ecolor ) ) {
			matInfo->mMaterial.setEmission( ai::get( ecolor ) );
			app::console() << " emission: " << ai::get( ecolor ) << std::endl;
		}
		if ( AI_SUCCESS == mtl->Get( AI_MATKEY_COLOR_TRANSPARENT, tcolor ) ) {
			matInfo->mTransparentColor =  ai::get( tcolor );
			app::console() << " transparent: " << ai::get( tcolor ) << std::endl;
		}
		
		// Load Textures
		int texIndex = 0;
		aiString texPath;
		
		// TODO: handle other aiTextureTypes
		if ( AI_SUCCESS == mtl->GetTexture( aiTextureType_DIFFUSE, texIndex, &texPath ) ) {
			app::console() << " diffuse texture " << texPath.data;
			fs::path texFsPath( texPath.data );
			fs::path modelFolder = modelPath.parent_path();
			fs::path realPath;
			if( rootPath.empty() ) {
				realPath = modelFolder / texFsPath;
			} else {
				realPath = rootPath / texFsPath;
			}
			
			int texFlag;
			if ( AI_SUCCESS == mtl->Get( AI_MATKEY_TEXFLAGS(aiTextureType_DIFFUSE, 0), texFlag ) ) {
				if( texFlag == aiTextureFlags_UseAlpha ) {
					matInfo->mUseAlpha = true;
					app::console() << " Texture uses alpha." << std::endl;
				}
			}
			
			app::console() << " [" << realPath.string() << "]" << std::endl;
			
			// texture wrap
			gl::Texture::Format format;
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
				matInfo->mTexture = gl::Texture::loadDds( loadFile( realPath )->createStream(), format );
				if ( !matInfo->mTexture )
					app::console() << "failed to laod dds..." << std::endl;
			} else {
				matInfo->mTexture = gl::Texture( loadImage( realPath ), format );
			}
		}
	}
	
	void loadBoneWeights( const aiMesh* aimesh, const Skeleton* skeleton, std::vector<BoneWeights>* boneWeights  )
	{
		unsigned int nbBones = aimesh->mNumBones;
		std::string name = ai::get( aimesh->mName );
		
		// Create a list of empty bone weights mirroring the # of vertices
		for( unsigned v=0; v < aimesh->mNumVertices; ++v ) {
			boneWeights->push_back( BoneWeights() );
		}
		
		for( unsigned b=0; b < nbBones; ++b ){
			NodeRef bone = skeleton->getBone( ai::get( aimesh->mBones[b]->mName ) );
			
			// Set the bone offset matrix if it hasn't been already
			if( bone->getOffset() == nullptr ) {
				bone->setOffsetMatrix( ai::get( aimesh->mBones[b]->mOffsetMatrix ) ) ;
			}
			
			// Add the bone weight information to the correct vertex index
			aiBone* aibone = aimesh->mBones[b];
			for(int w=0; w<aibone->mNumWeights; ++w ) {
				float weight = aibone->mWeights[w].mWeight;
				int boneWeightIndex = int( aibone->mWeights[w].mVertexId );
				(*boneWeights)[boneWeightIndex].addWeight( bone, weight );
			}
		}
	}
	
	Matrix44f getDefaultTransformation( std::string name, const aiScene* aiscene, Skeleton* skeleton )
	{
		const aiNode* ainode = ai::findMeshNode( name, aiscene, aiscene->mRootNode );
		if( ainode ) {
			std::string ainame = ai::get( ainode->mName );
			const Matrix44f& t = skeleton->getNode(ainame)->getInitialAbsoluteTransformation();
			return t;
		}
		return Matrix44f::identity();
	}

}

ModelSourceAssimp::ModelSourceAssimp( const fs::path& modelPath, const fs::path& rootAssetFolderPath )
{
	mModelPath = modelPath;
	mRootAssetFolderPath = rootAssetFolderPath;
	
	mImporter = std::unique_ptr<Assimp::Importer>( new Assimp::Importer() );
	mImporter->SetIOHandler( new CustomIOSystem() );
	mAiScene = mImporter->ReadFile( mModelPath.string(), ai::flags );
	
	//TODO: make own exception class to catch
	if( !mAiScene ) {
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

ModelSourceAssimpRef ModelSourceAssimp::create( const fs::path& modelPath, const fs::path& rootAssetFolderPath )
{
	return ModelSourceAssimpRef( new ModelSourceAssimp( modelPath, rootAssetFolderPath ) );
}

size_t getTotal( const std::vector<size_t> v )
{
	size_t total = 0;
	for(int i=0; i< v.size(); ++i ) {
		total += v[i];
	}
	return total;
}

size_t ModelSourceAssimp::getNumVertices( int section ) const
{
	if( section == -1 ) {
		return getTotal( mModelInfo.mNumVertices );
	}
	
	assert( section >= 0 && section < mModelInfo.mNumVertices.size() );
	return mModelInfo.mNumVertices[section];
}
size_t ModelSourceAssimp::getNumIndices( int section )  const
{
	if( section == -1 ) {
		return getTotal( mModelInfo.mNumIndices );
	}
	
	assert( section >= 0 && section < mModelInfo.mNumIndices.size() );
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
	
	for(int i=0; i< mAiScene->mNumMeshes; ++i ) {
		const aiMesh* aimesh = mAiScene->mMeshes[i];
		std::string name = ai::get( mAiScene->mMeshes[ i ]->mName );
		
		app::console() << "loading mesh " << i;
		if ( name != "" )
			app::console() << " [" << name << "]";
		app::console() << " #faces: " << aimesh->mNumFaces;
		app::console() << " #vertices: " << aimesh->mNumVertices << std::endl;
		
		target->setActiveSection( i );
		
		std::vector<Vec3f> positions;
		std::vector<uint32_t> indices;
		ai::loadPositions( aimesh, &positions );
		ai::loadIndices( aimesh, &indices );
		target->loadName( name );
		target->loadIndices( indices );
		target->loadVertexPositions( positions );
		
		if( mModelInfo.mHasNormals ) {
			std::vector<Vec3f> normals;
			ai::loadNormals( aimesh, &normals );
			target->loadVertexNormals( normals );
		}
		
		if( mModelInfo.mHasMaterials && aimesh->GetNumUVChannels() > 0 ) {
			std::vector<Vec2f> texCoords;
			MaterialInfo matInfo;
			ai::loadTexCoords( aimesh, &texCoords );
			ai::loadTexture(mAiScene, aimesh, &matInfo, mModelPath, mRootAssetFolderPath );
			target->loadTex( texCoords, matInfo );
		}
		
		if( mModelInfo.mHasSkeleton ) {
			if( loadSkeleton )
				target->loadSkeleton( skeleton );
			
			std::vector<BoneWeights> boneWeights;
			ai::loadBoneWeights( aimesh, skeleton.get(), &boneWeights );
			target->loadBoneWeights( boneWeights );
		}		
	}
}
