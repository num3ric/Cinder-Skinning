//
//  SkinnedVboMesh.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-05.
//
//

#pragma once

#include "AMeshSection.h"

#include "cinder/app/App.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"

#include <vector>

namespace model {

typedef std::shared_ptr<class SkinnedVboMesh> SkinnedVboMeshRef;

class SkinnedVboMesh
{
public:
	static const int MAXBONES = 92;
	
	struct MeshSection : public AMeshSection
	{
		MeshSection();
		void updateMesh( bool enableSkinning = true ) override;
		
		ci::gl::VboMesh&		getVboMesh() { return mVboMesh; }
		const ci::gl::VboMesh&	getVboMesh() const { return mVboMesh; }
		void				setVboMesh( size_t numVertices, size_t numIndices, ci::gl::VboMesh::Layout layout, GLenum primitiveType );
		
		std::array<ci::Matrix44f, MAXBONES>* mBoneMatricesPtr;
		std::array<ci::Matrix44f, MAXBONES>* mInvTransposeMatricesPtr;
	private:
		ci::gl::VboMesh mVboMesh;
	};
	typedef std::shared_ptr< struct SkinnedVboMesh::MeshSection > MeshVboSectionRef;
	
	static SkinnedVboMeshRef create( ModelSourceRef modelSource, std::shared_ptr<Skeleton> skeleton = nullptr, ci::gl::GlslProgRef skinningShader = nullptr );
	
	void update();
	
	MeshVboSectionRef&						getActiveSection() { return mActiveSection; }
	const MeshVboSectionRef&				getActiveSection() const { return mActiveSection; }
	MeshVboSectionRef&						setActiveSection( int index );
	std::vector< MeshVboSectionRef >&		getSections() { return mMeshSections; }
	const std::vector< MeshVboSectionRef >&	getSections() const { return mMeshSections; }
	
	std::shared_ptr<Skeleton>&			getSkeleton() { return mActiveSection->getSkeleton(); }
	const std::shared_ptr<Skeleton>&	getSkeleton() const { return mActiveSection->getSkeleton(); }
	void								setSkeleton( const std::shared_ptr<Skeleton>& skeleton ) { mActiveSection->setSkeleton( skeleton ); }
	bool								hasSkeleton() const { return mActiveSection->getSkeleton() != nullptr; }
	
	void						setDefaultTransformation( const ci::Matrix44f& transformation ) { mActiveSection->setDefaultTransformation( transformation ); }
	const ci::Matrix44f&		getDefaultTranformation() const { return mActiveSection->getDefaultTranformation(); }
	
	ci::gl::GlslProgRef getShader() { return mSkinningShader; }
	const ci::gl::GlslProgRef getShader() const { return mSkinningShader; }
	
	void setEnableSkinning( bool enabled ) { mEnableSkinning = enabled; }
	
	friend struct SkinnedVboMesh::MeshSection;
	
	std::array<ci::Matrix44f, MAXBONES> mBoneMatrices;
	std::array<ci::Matrix44f, MAXBONES> mInvTransposeMatrices;
	ci::gl::GlslProgRef mSkinningShader;
protected:
	bool mEnableSkinning;
	SkinnedVboMesh( ModelSourceRef modelSource, ci::gl::GlslProgRef skinningShader, std::shared_ptr<Skeleton> skeleton = nullptr );
	MeshVboSectionRef mActiveSection;
	std::vector< MeshVboSectionRef > mMeshSections;
};


typedef std::shared_ptr< struct SkinnedVboMesh::MeshSection > MeshVboSectionRef;
	
} //end namespace model
