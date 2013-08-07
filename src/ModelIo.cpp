
#include "ModelIo.h"
#include "Node.h"
#include "ModelSourceAssimp.h"

namespace model {

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
			
			if( sum < 1.0f - ci::EPSILON && sum !=0 ) {
				for (int i=0; i < NB_WEIGHTS; ++i) {
					mWeights[i] *= 1.0f / sum;
				}
			}
		}
	}
}


void ModelTarget::setActiveSection( int index ) { }

void ModelTarget::loadName( std::string name ) { }

void ModelTarget::loadTex( const std::vector<ci::Vec2f>& texCoords, const MaterialInfo& texInfo ) { }

void ModelTarget::loadVertexNormals( const std::vector<ci::Vec3f>& normals ) { }

void ModelTarget::loadSkeleton( const std::shared_ptr<Skeleton>& skeleton ) { }

void ModelTarget::loadBoneWeights( const std::vector<BoneWeights>& boneWeights ) { }

void ModelTarget::loadDefaultTransformation( const ci::Matrix44f& transformation ) { }
	
ModelSourceAssimpRef loadModel( const ci::DataSourceRef dataSource )
{
	ci::fs::path relativePath = dataSource->getFilePath();
	if( relativePath.empty() )
		relativePath = ci::fs::path( dataSource->getFilePathHint() );
	return ModelSourceAssimp::create( relativePath );
}

ModelSourceAssimpRef loadModel( const ci::fs::path& modelPath, const ci::fs::path& rootAssetFolderPath )
{

	return ModelSourceAssimp::create( modelPath, rootAssetFolderPath );
}

ModelIoException::ModelIoException( const std::string &message ) throw()
//: ModelIoException() no constructor delegation in VS2012 :(
{
	mMessage[0] = 0;
	strncpy( mMessage, message.c_str(), 255 );
}

} //end namespace model