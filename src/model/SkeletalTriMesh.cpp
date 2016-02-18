//
//  SkinnedModel.cpp
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-04.
//
//

#include "SkeletalTriMesh.h"
#include "Node.h"
#include "Skeleton.h"

using namespace ci;
using namespace model;
	
SkeletalTriMesh::Section::Section( const SectionSourceRef& source, const std::vector<Weights>& weights )
: AMeshSection( source )
, mTriMesh( TriMesh::create( *source ) )
, mWeights( weights )
{
	auto verts =  mTriMesh->getPositions<3>();
	for( size_t i=0; i<mTriMesh->getNumVertices(); ++i ) {
		mInitialPositions.emplace_back( *verts );
		verts++;
	}
	
	mInitialNormals = mTriMesh->getNormals();
}

void SkeletalTriMesh::Section::reset()
{
	*mTriMesh->getPositions<3>() = *mInitialPositions.data();
	if( mTriMesh->hasNormals() )
		mTriMesh->getNormals() = mInitialNormals;
}

void SkeletalTriMesh::Section::update()
{
	CI_ASSERT( mWeights.size() == mTriMesh->getNumVertices() );
	
	int vertexIdx = 0;
	for( const auto& weights : mWeights ) {
		
		vec3 weightedPosition;
		vec3 weightedNormal;
		for(  unsigned int i=0; i < weights.getNumActiveWeights(); ++i ) {
			
			const vec3& srcPos		= mInitialPositions.at( vertexIdx );
			auto bone				= weights.getBone( i );
			auto skinningMatrix		= bone->getAbsoluteTransformation() * *bone->getOffset();
			float weight			= weights.getWeight( i );
			
			weightedPosition += weight * vec3( skinningMatrix * vec4( srcPos, 1 ) );
			if( mTriMesh->hasNormals() )
				weightedNormal += weight * vec3( skinningMatrix * vec4( mInitialNormals[ vertexIdx ], 1 ) );
		}
		
		mTriMesh->getPositions<3>()[vertexIdx] = weightedPosition;
		if( mTriMesh->hasNormals() )
			mTriMesh->getNormals().at( vertexIdx ) = weightedNormal;
		
		++vertexIdx;
	}
}

SkeletalTriMeshRef SkeletalTriMesh::create( const model::Source& modelSource, SkeletonRef skeleton )
{
	return SkeletalTriMeshRef( new SkeletalTriMesh( modelSource, skeleton ) );
}

SkeletalTriMesh::SkeletalTriMesh( const model::Source& modelSource, SkeletonRef skeleton )
: Actor( modelSource, skeleton )
{
	for( auto& sectionSource : modelSource.getSectionSources() ) {
		SectionRef section{ new Section{ sectionSource, sectionSource->getWeights() } };
		mSections.push_back( section );
	}
}

void SkeletalTriMesh::update()
{
	for( auto& section : mSections ) {
		section->update();
	}
}

