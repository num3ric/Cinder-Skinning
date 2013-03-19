//
//  SkinnedVboMesh.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-05.
//
//

#pragma once

#include "Skeleton.h"
#include "ASkinnedMesh.h"
#include "ModelIo.h"

#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"

using namespace ci;

class Skeleton;

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
		
		gl::GlslProg		getShader() { return mSkinningShader; }
		const gl::GlslProg& getShader() const { return mSkinningShader; }
		
		gl::VboMesh&		getVboMesh() { return mVboMesh; }
		const gl::VboMesh&	getVboMesh() const { return mVboMesh; }
		void				setVboMesh( size_t numVertices, size_t numIndices, gl::VboMesh::Layout layout, GLenum primitiveType );
		
		std::array<Matrix44f, MAXBONES>* boneMatrices;
		std::array<Matrix44f, MAXBONES>* invTransposeMatrices;
	private:
		gl::VboMesh mVboMesh;
		gl::GlslProg mSkinningShader;
	};
	typedef std::shared_ptr< struct SkinnedVboMesh::MeshSection > MeshVboSectionRef;
	
	SkinnedVboMesh( ModelSourceRef modelSource, const SkeletonRef& skeleton = nullptr );
	
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
	
	friend class SkinnedVboMesh::MeshSection;
	
	std::array<Matrix44f, MAXBONES> mBoneMatrices;
	std::array<Matrix44f, MAXBONES> mInvTransposeMatrices;
protected:
	MeshVboSectionRef mActiveSection;
	std::vector< MeshVboSectionRef > mMeshSections;
};


typedef std::shared_ptr< struct SkinnedVboMesh::MeshSection > MeshVboSectionRef;
