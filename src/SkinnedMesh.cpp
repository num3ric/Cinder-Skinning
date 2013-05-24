//
//  SkinnedModel.cpp
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-04.
//
//

#include "SkinnedMesh.h"
#include "ModelTargetSkinnedMesh.h"
#include "Node.h"
#include "Skeleton.h"

namespace model {

void SkinnedMesh::MeshSection::updateMesh( bool enableSkinning )
{
	if( enableSkinning ) {
		int vertexId = 0;
		for( const BoneWeights& boneWeights : getBoneWeights() ) {
			mTriMesh.getVertices()[vertexId] = ci::Vec3f::zero();
			if( hasNormals() )
				mTriMesh.getNormals()[vertexId] = ci::Vec3f::zero();
			
			for(  unsigned int i=0; i < boneWeights.mActiveNbWeights; ++i ) {
				const ci::Vec3f& srcPos = mInitialPositions[vertexId];
				ci::Vec3f srcNorm;
				if( hasNormals() )
					srcNorm = mInitialNormals[vertexId];
				NodeRef bone = boneWeights.getBone(i);
				ci::Matrix44f skinningTransfo = bone->getAbsoluteTransformation() * *bone->getOffset();
				float weight = boneWeights.getWeight(i);
				
				mTriMesh.getVertices()[vertexId] += weight * (skinningTransfo * srcPos);
				
				if( hasNormals() )
					mTriMesh.getNormals()[vertexId] += weight * (skinningTransfo.subMatrix33(0, 0) * srcNorm);
			}
			++vertexId;
		}
		mIsAnimated = true;
	} else if( mIsAnimated ) {
		mTriMesh.getVertices() = mInitialPositions;
		if( hasNormals() )
			mTriMesh.getNormals() = mInitialNormals;
		mIsAnimated = false;
	}
}

SkinnedMeshRef SkinnedMesh::create( ModelSourceRef modelSource, SkeletonRef skeleton )
{
	return SkinnedMeshRef( new SkinnedMesh( modelSource, skeleton ) );
}

SkinnedMesh::SkinnedMesh( ModelSourceRef modelSource, SkeletonRef skeleton )
: mEnableSkinning( true )
{
	assert( modelSource->getNumSections() > 0 );
	
	for( unsigned int i = 0; i< modelSource->getNumSections(); ++i ) {
		MeshSectionRef section = std::make_shared<SkinnedMesh::MeshSection>();
		section->setSkeleton( skeleton );
		mMeshSections.push_back( section );
	}
	mActiveSection = mMeshSections[0];
	
	ModelTargetSkinnedMesh target( this );
	modelSource->load( &target );
}

void SkinnedMesh::update()
{
	for( MeshSectionRef section : mMeshSections ) {
		section->updateMesh( mEnableSkinning );
	}
}

MeshSectionRef& SkinnedMesh::setActiveSection( int index )
{
	assert( index >= 0 && index < (int) mMeshSections.size() );
	mActiveSection = mMeshSections[ index ];
	return mActiveSection;
}

void SkinnedMesh::appendSection( const MeshSectionRef& meshSection )
{
	mMeshSections.push_back( meshSection );
	mActiveSection = meshSection;
}

} //end namespace model