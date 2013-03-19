
#include "ModelIo.h"
#include "Node.h"

void BoneWeights::addWeight( const NodeRef& bone, float weight ) {
	if( mActiveNbWeights == NB_WEIGHTS ) {
		// TODO: we should warn the user here?
		return;
	} else {
		mWeights[mActiveNbWeights] = weight;
		mBones[mActiveNbWeights] = bone;
		++mActiveNbWeights;
		
		// If we have NB_WEIGHTS bone weights and their sum isn't 1.0,
		// we renormalize the weights.
		if( mActiveNbWeights == NB_WEIGHTS ) {
			float sum = 0.0f;
			for (int i=0; i < NB_WEIGHTS; ++i) {
				sum += mWeights[i];
			}
			
			if( sum < 1.0f - EPSILON & sum !=0 ) {
				for (int i=0; i < NB_WEIGHTS; ++i) {
					mWeights[i] *= 1.0f / sum;
				}
			}
		}
	}
}


void ModelTarget::setActiveSection( int index ) { }

void ModelTarget::loadName( std::string name ) { }

void ModelTarget::loadTex( const std::vector<Vec2f>& texCoords, const MaterialInfo& texInfo ) { }

void ModelTarget::loadVertexNormals( const std::vector<Vec3f>& normals ) { }

void ModelTarget::loadSkeleton( const std::shared_ptr<Skeleton>& skeleton ) { }

void ModelTarget::loadBoneWeights( const std::vector<BoneWeights>& boneWeights ) { }


#include "ModelSourceAssimp.h"

ModelSourceAssimpRef loadModel( const fs::path& modelPath, const fs::path& rootAssetFolderPath )
{
	return ModelSourceAssimp::create( modelPath, rootAssetFolderPath );
}