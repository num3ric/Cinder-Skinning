#include "MorphedMesh.h"
#include "ModelIo.h"

#include "Skeleton.h"
#include "Renderer.h"
#include "Resources.h"

#include "cinder/Utilities.h"

using namespace ci;
using namespace model;

MorphedMesh::Section::Section( const SectionSourceRef& source, size_t numMorphTargets, ci::gl::GlslProgRef shader, gl::Batch::AttributeMapping mapping )
: ABatchSection( source, shader, mapping )
, mNumMorphTargets( numMorphTargets )
{
}

MorphedMeshRef MorphedMesh::create( const model::Source& source, ci::gl::GlslProgRef shader )
{
	if( ! shader )
		shader = model::Renderer::instance().getShader( MeshType::MORPHED );

	return MorphedMeshRef( new MorphedMesh( source, shader ) );
}

void MorphedMesh::setMorphTargetWeight( size_t index, float weightContribution )
{
	mMorphTargetWeights.at( index ) = weightContribution;
}

MorphedMesh::MorphedMesh( const model::Source& source, ci::gl::GlslProgRef shader )
: mMorphTargetWeights{}
{
	size_t index = 0;
	for( auto& sectionSource : source.getSectionSources() ) {
		
		size_t numMorphTargets = sectionSource->getNumMorphTargets();
		
		gl::Batch::AttributeMapping mapping;
		for( size_t i = 0; i < sectionSource->getNumMorphTargets(); ++i ) {
			mapping[static_cast<geom::Attrib>( size_t( geom::Attrib::CUSTOM_0 ) + i )] = "vPositionDiff" + toString( i );
		}
		
		SectionRef section{ new Section{ sectionSource, numMorphTargets, shader, mapping } };
		mMeshSections.emplace_back( section );
		
		mMorphTargetWeights.at( index ) = (index == 0) ? 1.0f : 0.0f;
		++index;
	}
}
