#pragma once

#include "ASkinnedMesh.h"

#include "cinder/TriMesh.h"

#include <vector>
#include <string>

class model::Skeleton;

namespace model {

typedef std::shared_ptr< class SkinnedMesh > SkinnedMeshRef;

class SkinnedMesh
{
public:
	struct MeshSection : public ASkinnedMesh
	{
		MeshSection() : ASkinnedMesh() { }
		void updateMesh( float time, bool enableSkinning = true ) override;
		void drawMesh() override;
		
		std::string mName;
		ci::TriMesh mTriMesh;
		std::vector< ci::Vec3f > mInitialPositions;
		std::vector< ci::Vec3f > mInitialNormals;
	};
	typedef std::shared_ptr< struct SkinnedMesh::MeshSection > MeshSectionRef;
	
	static SkinnedMeshRef create( ModelSourceRef modelSource, std::shared_ptr<Skeleton> skeleton = nullptr );
	void appendSection( const MeshSectionRef& meshSection );
	void update( float time, bool enableSkinning = true );
	void draw();
	
	std::shared_ptr<Skeleton>&		getSkeleton() { return mActiveSection->getSkeleton(); }
	const std::shared_ptr<Skeleton>&	getSkeleton() const { return mActiveSection->getSkeleton(); }
	
	MeshSectionRef&			getActiveSection() { return mActiveSection; }
	const MeshSectionRef&	getActiveSection() const { return mActiveSection; }
	MeshSectionRef&			setActiveSection( int index );
	std::vector< MeshSectionRef >&			getSections() { return mMeshSections; }
	const std::vector< MeshSectionRef >&	getSections() const { return mMeshSections; }
	
protected:
	SkinnedMesh( ModelSourceRef modelSource, std::shared_ptr<Skeleton> skeleton = nullptr );
	//add full behavior/functions relating to "active section"
	MeshSectionRef mActiveSection;
	std::vector< MeshSectionRef > mMeshSections;
};

typedef std::shared_ptr< struct SkinnedMesh::MeshSection > MeshSectionRef;

} //end namespace model