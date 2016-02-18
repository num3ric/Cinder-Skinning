#pragma once

#include "AMeshSection.h"

#include <vector>

namespace model {

	typedef std::shared_ptr<class StaticMesh> StaticMeshRef;

	class StaticMesh {
	public:
		class Section : public ABatchSection
		{
			friend class StaticMesh;
		public:
			Section( const SectionSourceRef& source, ci::gl::GlslProgRef shader );
		};
		typedef std::shared_ptr<Section> SectionRef;

		static StaticMeshRef create( const model::Source& source, ci::gl::GlslProgRef shader = nullptr );
		
		virtual ~StaticMesh() { };
		
		const std::vector<SectionRef>&	getSections() const { return mMeshSections; }
	protected:
		StaticMesh( const model::Source& source, ci::gl::GlslProgRef shader );
		
		std::vector<StaticMesh::SectionRef>	mMeshSections;
	};
}