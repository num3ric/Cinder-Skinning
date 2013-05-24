
#pragma once
#include "cinder/gl/GlslProg.h"

#include "ASkinnedMesh.h"
#include "Node.h"

class ci::CameraPersp;

namespace model {
	
	class SkinnedMesh;
	class SkinnedVboMesh;
	class Skeleton;
	
	typedef std::shared_ptr<class SkinningRenderer> SkinningRendererRef;
	
	class SkinningRenderer {
	public:
		// TODO: convert into singleton?
		// TODO: Make draw section call with material binding optional (for custom uses)
		
		// custom shader still needs uniforms
		static SkinningRendererRef create();
		SkinningRenderer(const SkinningRenderer& that) = delete;
		SkinningRenderer& operator=(const SkinningRenderer&) = delete;
		
		ci::gl::GlslProgRef			getShader() { return mSkinningShader; }
		const ci::gl::GlslProgRef	getShader() const { return mSkinningShader; }
		
		void	draw( std::shared_ptr<SkinnedMesh> skinnedMesh ) const;
		void	draw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh ) const;
		void	draw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" ) const;
		//! Render the node names.
		void	drawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera ) const;
	private:
		SkinningRenderer();
		
		void drawSection( const ASkinnedMesh& section, std::function<void()> drawMesh ) const;
		//! Determines whether the node should be rendered as part of the skeleton.
		bool	isVisibleNode( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node ) const;
		//! Draw the visible nodes/bones of the skeleton by traversing recursively its node transformation hierarchy.
		void	drawRelative( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node, const std::shared_ptr<Node>& parent = nullptr ) const;
		//! Draw the visible nodes/bones of the skeleton by using its absolute bone positions.
		void	drawAbsolute( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node ) const;
		
		
		void drawConnected( const ci::Vec3f& nodePos, const ci::Vec3f& parentPos ) const;
		void drawJoint( const ci::Vec3f& nodePos ) const;
		//! Optional argument for precomputed distance
		void drawBone( const ci::Vec3f& start, const ci::Vec3f& end, float dist = -1.0f ) const;
		void drawSkeletonNode( const Node& node, Node::RenderMode mode = Node::RenderMode::CONNECTED ) const;
		void drawSkeletonNodeRelative( const Node& node, Node::RenderMode mode = Node::RenderMode::CONNECTED ) const;
		void drawLabel( const Node& node, const ci::CameraPersp& camera, const ci::Matrix44f& mv ) const;
		
		ci::gl::GlslProgRef mSkinningShader;
	};
}