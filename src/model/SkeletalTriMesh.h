#pragma once

#include "cinder/TriMesh.h"

#include <vector>
#include <string>

#include "AMeshSection.h"
#include "Actor.h"

namespace model {

typedef std::shared_ptr< class SkeletalTriMesh > SkeletalTriMeshRef;

class SkeletalTriMesh : public Actor
{
public:
	class Section : public AMeshSection
	{
		friend class SkeletalTriMesh;
	public:
		void reset();
		void update();
		
		ci::TriMeshRef		getTriMesh() const { return mTriMesh; }
		const std::vector<glm::vec3>& getInitialPositions() const { return mInitialPositions; }
		const std::vector<glm::vec3>& getInitialNormals() const { return mInitialNormals; }
	private:
		Section( const SectionSourceRef& source, const std::vector<Weights>& weights );
		
		ci::TriMeshRef			mTriMesh;
		std::vector<Weights>	mWeights;
		std::vector<glm::vec3>	mInitialPositions;
		std::vector<glm::vec3>	mInitialNormals;
	};
	typedef std::shared_ptr<Section> SectionRef;
	
	static SkeletalTriMeshRef create( const model::Source& modelSource, SkeletonRef skeleton = nullptr );
	
	//! Updates the mesh vertices (and normals if needed) based on the current skeleton pose.
	void update();

	const std::vector<SectionRef>&	getSections() const { return mSections; }
protected:
	SkeletalTriMesh( const model::Source& modelSource, SkeletonRef skeleton = nullptr );

	std::vector<SectionRef>	mSections;
};

} //end namespace model