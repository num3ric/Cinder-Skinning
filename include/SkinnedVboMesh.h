//
//  SkinnedVboMesh.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-05.
//
//

#pragma once

#include "ASkinnedMesh.h"

#include "cinder/app/App.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"

#include <vector>

class Skeleton;

namespace model {

typedef std::shared_ptr<class SkinnedVboMesh> SkinnedVboMeshRef;

class SkinnedVboMesh
{
public:
	static const int MAXBONES = 92;
	
	struct MeshSection : public ASkinnedMesh
	{
		MeshSection();
		void updateMesh( float time, bool enableSkinning = true ) override;
		void drawMesh() override;
		
		ci::gl::GlslProg		getShader() { return mSkinningShader; }
		const ci::gl::GlslProg& getShader() const { return mSkinningShader; }
		
		ci::gl::VboMesh&		getVboMesh() { return mVboMesh; }
		const ci::gl::VboMesh&	getVboMesh() const { return mVboMesh; }
		void				setVboMesh( size_t numVertices, size_t numIndices, ci::gl::VboMesh::Layout layout, GLenum primitiveType );
		
		std::array<ci::Matrix44f, MAXBONES>* boneMatrices;
		std::array<ci::Matrix44f, MAXBONES>* invTransposeMatrices;
	private:
		ci::gl::VboMesh mVboMesh;
		ci::gl::GlslProg mSkinningShader;
	};
	typedef std::shared_ptr< struct SkinnedVboMesh::MeshSection > MeshVboSectionRef;
	
	static SkinnedVboMeshRef create( ModelSourceRef modelSource, const std::shared_ptr<Skeleton>& skeleton = nullptr );
	
	void update( float time, bool enableSkinning = true );
	void draw();
	
	MeshVboSectionRef&						getActiveSection() { return mActiveSection; }
	const MeshVboSectionRef&				getActiveSection() const { return mActiveSection; }
	MeshVboSectionRef&						setActiveSection( int index );
	std::vector< MeshVboSectionRef >&		getSections() { return mMeshSections; }
	const std::vector< MeshVboSectionRef >&	getSections() const { return mMeshSections; }
	
	std::shared_ptr<Skeleton>&			getSkeleton() { return mActiveSection->getSkeleton(); }
	const std::shared_ptr<Skeleton>&	getSkeleton() const { return mActiveSection->getSkeleton(); }
	void								setSkeleton( const std::shared_ptr<Skeleton>& skeleton ) { mActiveSection->setSkeleton( skeleton ); }
	bool								hasSkeleton() const { return mActiveSection->getSkeleton() != nullptr; }
	
	friend struct SkinnedVboMesh::MeshSection;
	
	std::array<ci::Matrix44f, MAXBONES> mBoneMatrices;
	std::array<ci::Matrix44f, MAXBONES> mInvTransposeMatrices;
protected:
	SkinnedVboMesh( ModelSourceRef modelSource, const std::shared_ptr<Skeleton>& skeleton = nullptr );
	MeshVboSectionRef mActiveSection;
	std::vector< MeshVboSectionRef > mMeshSections;
};


typedef std::shared_ptr< struct SkinnedVboMesh::MeshSection > MeshVboSectionRef;
	
} //end namespace model
