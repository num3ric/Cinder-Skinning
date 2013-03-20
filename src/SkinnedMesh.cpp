//
//  SkinnedModel.cpp
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-04.
//
//

#include "SkinnedMesh.h"
#include "ModelTargetSkinnedMesh.h"

void SkinnedMesh::MeshSection::updateMesh( float time, bool enableSkinning )
{
	if (enableSkinning) {
		int vertexId = 0;
		for( const BoneWeights& boneWeights : getBoneWeights() ) {
			mTriMesh.getVertices()[vertexId] = Vec3f::zero();
			if( hasNormals() )
				mTriMesh.getNormals()[vertexId] = Vec3f::zero();
			
			for( int i=0; i < boneWeights.mActiveNbWeights; ++i ) {
				const Vec3f& srcPos = mInitialPositions[vertexId];
				Vec3f srcNorm;
				if( hasNormals() )
					srcNorm = mInitialNormals[vertexId];
				NodeRef bone = boneWeights.getBone(i);
				Matrix44f skinningTransfo = bone->getAbsoluteTransformation() * *bone->getOffset();
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


void SkinnedMesh::MeshSection::drawMesh()	
{
	gl::draw( mTriMesh );	
}

SkinnedMeshRef SkinnedMesh::create( ModelSourceRef modelSource, const SkeletonRef& skeleton )
{
	return SkinnedMeshRef( new SkinnedMesh( modelSource, skeleton ) );
}

SkinnedMesh::SkinnedMesh( ModelSourceRef modelSource, const SkeletonRef& skeleton )
{
	assert( modelSource->getNumSections() > 0 );
	
	for(int i = 0; i< modelSource->getNumSections(); ++i ) {
		mMeshSections.push_back( std::make_shared<SkinnedMesh::MeshSection>() );
	}
	mActiveSection = mMeshSections[0];
	mActiveSection->setSkeleton( skeleton );
	
	ModelTargetSkinnedMesh target( this );
	modelSource->load( &target );
}

void SkinnedMesh::update( float time, bool enableSkinning )
{
	mActiveSection->getSkeleton()->update( time );
	for( MeshSectionRef section : mMeshSections ) {
		section->updateMesh( time, enableSkinning );
	}
}

MeshSectionRef& SkinnedMesh::setActiveSection( int index )
{
	assert( index >= 0 && index < mMeshSections.size() );
	mActiveSection = mMeshSections[ index ];
	return mActiveSection;
}

void SkinnedMesh::appendSection( const MeshSectionRef& meshSection )
{
	mMeshSections.push_back( meshSection );
	mActiveSection = meshSection;
}

void SkinnedMesh::draw()
{	
	for( const MeshSectionRef& section : mMeshSections ) {
		section->draw();
	}
}