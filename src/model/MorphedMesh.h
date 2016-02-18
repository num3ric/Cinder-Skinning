#pragma once

#include "AMeshSection.h"

#include <vector>

namespace model {

	typedef std::shared_ptr<class MorphedMesh> MorphedMeshRef;

	class MorphedMesh {
	public:
		const static int MAXMORPHTARGETS = 10;
		
		class Section : public ABatchSection
		{
			friend class MorphedMesh;
		public:
			Section( const SectionSourceRef& source, size_t numMorphTargets, ci::gl::GlslProgRef shader, ci::gl::Batch::AttributeMapping mapping );
		private:
			size_t				mNumMorphTargets;
		};
		typedef std::shared_ptr<Section> SectionRef;

		static MorphedMeshRef create( const model::Source& source, ci::gl::GlslProgRef shader = nullptr );
		
		void	setMorphTargetWeight( size_t index, float weightContribution );
		float	getMorphTargetWeight( size_t index ) { return mMorphTargetWeights.at( index ); }
		const std::array<float, 10> getMorphTargetWeights() const { return mMorphTargetWeights; }
		
		const std::vector<SectionRef>&	getSections() const { return mMeshSections; }
	protected:
		MorphedMesh( const model::Source& source, ci::gl::GlslProgRef shader );
		
		std::vector<SectionRef>	mMeshSections;
		std::array<float, MAXMORPHTARGETS>	mMorphTargetWeights;
	};
}