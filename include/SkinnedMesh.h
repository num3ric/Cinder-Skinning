#pragma once

#include "ASkinnedMesh.h"
#include "Skeleton.h"
#include "Node.h"

//#include "cinder/Cinder.h"
#include "cinder/TriMesh.h"

#include <vector>
#include <string>

using namespace ci;

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
		TriMesh mTriMesh;
		std::vector< Vec3f > mInitialPositions;
		std::vector< Vec3f > mInitialNormals;
	};
	typedef std::shared_ptr< struct SkinnedMesh::MeshSection > MeshSectionRef;
	
	static SkinnedMeshRef create( ModelSourceRef modelSource, const SkeletonRef& skeleton = nullptr );
	void appendSection( const MeshSectionRef& meshSection );
	void update( float time, bool enableSkinning = true );
	void draw();
	
	SkeletonRef&		getSkeleton() { return mActiveSection->getSkeleton(); }
	const SkeletonRef&	getSkeleton() const { return mActiveSection->getSkeleton(); }
	
	MeshSectionRef&			getActiveSection() { return mActiveSection; }
	const MeshSectionRef&	getActiveSection() const { return mActiveSection; }
	MeshSectionRef&			setActiveSection( int index );
	std::vector< MeshSectionRef >&			getSections() { return mMeshSections; }
	const std::vector< MeshSectionRef >&	getSections() const { return mMeshSections; }
	
protected:
	SkinnedMesh( ModelSourceRef modelSource, const SkeletonRef& skeleton = nullptr );
	//add full behavior/functions relating to "active section"
	MeshSectionRef mActiveSection;
	std::vector< MeshSectionRef > mMeshSections;
};

typedef std::shared_ptr< struct SkinnedMesh::MeshSection > MeshSectionRef;