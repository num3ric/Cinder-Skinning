#pragma once

#include "AMeshSection.h"

#include "cinder/TriMesh.h"

#include <vector>
#include <string>

namespace model {

typedef std::shared_ptr< class SkinnedMesh > SkinnedMeshRef;

class SkinnedMesh
{
public:
	struct MeshSection : public AMeshSection
	{
		MeshSection() : AMeshSection() { }
		void updateMesh( bool enableSkinning = true ) override;
		
		std::string mName;
		ci::TriMesh mTriMesh;
		std::vector< ci::Vec3f > mInitialPositions;
		std::vector< ci::Vec3f > mInitialNormals;
	};
	typedef std::shared_ptr< struct SkinnedMesh::MeshSection > MeshSectionRef;
	
	static SkinnedMeshRef create( ModelSourceRef modelSource, std::shared_ptr<Skeleton> skeleton = nullptr );
	void appendSection( const MeshSectionRef& meshSection );
	void update();
	
	std::shared_ptr<Skeleton>&			getSkeleton() { return mActiveSection->getSkeleton(); }
	const std::shared_ptr<Skeleton>&	getSkeleton() const { return mActiveSection->getSkeleton(); }
	bool								hasSkeleton() const { return mActiveSection->getSkeleton() != nullptr; }
	
	MeshSectionRef&			getActiveSection() { return mActiveSection; }
	const MeshSectionRef&	getActiveSection() const { return mActiveSection; }
	MeshSectionRef&			setActiveSection( int index );
	std::vector< MeshSectionRef >&			getSections() { return mMeshSections; }
	const std::vector< MeshSectionRef >&	getSections() const { return mMeshSections; }
	
	void						setDefaultTransformation( const ci::Matrix44f& transformation ) { mActiveSection->setDefaultTransformation( transformation ); }
	const ci::Matrix44f&		getDefaultTranformation() const { return mActiveSection->getDefaultTranformation(); }
	
	void setEnableSkinning( bool enabled ) { mEnableSkinning = enabled; }
protected:
	SkinnedMesh( ModelSourceRef modelSource, std::shared_ptr<Skeleton> skeleton = nullptr );
	//add full behavior/functions relating to "active section"
	bool mEnableSkinning;
	MeshSectionRef mActiveSection;
	std::vector< MeshSectionRef > mMeshSections;
};

typedef std::shared_ptr< struct SkinnedMesh::MeshSection > MeshSectionRef;

} //end namespace model