#include "StaticMesh.h"
#include "ModelIo.h"
#include "Renderer.h"

using namespace ci;
using namespace model;

StaticMesh::Section::Section( const SectionSourceRef& source, ci::gl::GlslProgRef shader )
: ABatchSection( source, shader )
{
	
}

StaticMeshRef StaticMesh::create( const model::Source& source, ci::gl::GlslProgRef shader )
{
	if( ! shader )
		shader = model::Renderer::instance().getShader( MeshType::STATIC );

	return StaticMeshRef( new StaticMesh( source, shader ) );
}

StaticMesh::StaticMesh( const model::Source& source, ci::gl::GlslProgRef shader )
{
	for( auto& sectionSource : source.getSectionSources() ) {
		SectionRef section{ new Section{ sectionSource, shader } };
		mMeshSections.emplace_back( section );
	}
}
