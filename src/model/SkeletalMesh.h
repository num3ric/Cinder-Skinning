//
//  SkeletalMesh.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-05.
//
//

#pragma once

#include <vector>

#include "AMeshSection.h"
#include "Actor.h"

namespace model {

typedef std::shared_ptr<class SkeletalMesh> SkeletalMeshRef;

class SkeletalMesh : public Actor
{
public:
	static const int MAXBONES = 92;
	
	class Section : public ABatchSection
	{
		friend class SkeletalMesh;
	public:
		const SkeletonRef&						getSkeleton() const { return mSkeleton; }
		const std::array<glm::mat4, MAXBONES>*	getBoneMatrices() const { return mBoneMatrices; }
		const std::array<glm::mat4, MAXBONES>*	getInvTransposeMatrices() const { return mInvTransposeMatrices; }
	private:
		Section( const SectionSourceRef& source, ci::gl::GlslProgRef shader );
		
		std::array<glm::mat4, MAXBONES>*		mBoneMatrices;
		std::array<glm::mat4, MAXBONES>*		mInvTransposeMatrices;
		SkeletonRef								mSkeleton;
	};
	
	typedef std::shared_ptr<Section> SectionRef;
	
	static SkeletalMeshRef create( const model::Source& modelSource, SkeletonRef skeleton = nullptr, ci::gl::GlslProgRef skinningShader = nullptr );
	
	//! Updates the bones matrices (used by skinning shaders) based on the current skeleton pose.
	void update() override;
	
	const std::vector<SectionRef>&			getSections() const { return mMeshSections; }
	
	const std::array<glm::mat4, MAXBONES>&	getBoneMatrices() const { return mBoneMatrices; }
	const std::array<glm::mat4, MAXBONES>&	getInvTransposeMatrices() const { return mInvTransposeMatrices; }
protected:
	SkeletalMesh( const model::Source& modelSource, SkeletonRef skeleton, ci::gl::GlslProgRef skinningShader );

	std::array<glm::mat4, MAXBONES>		mBoneMatrices;
	std::array<glm::mat4, MAXBONES>		mInvTransposeMatrices;
	std::vector<SectionRef>				mMeshSections;
};
	
} //end namespace model

