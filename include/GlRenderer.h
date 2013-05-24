
#pragma once
#include "cinder/gl/GlslProg.h"

#include "Renderer.h"
#include "ASkinnedMesh.h"
#include "Node.h"

class ci::CameraPersp;

namespace model {
	
	typedef std::shared_ptr<class GlRenderer> GlRendererRef;
	
	class GlRenderer : public Renderer {
	public:
		// TODO: convert into singleton?
		// TODO: Make draw section call with material binding optional (for custom uses)
		
		// custom shader still needs uniforms
		static GlRendererRef create( ci::gl::GlslProgRef customShader = nullptr );
		GlRenderer(const GlRenderer& that) = delete;
		GlRenderer& operator=(const GlRenderer&) = delete;
		
		ci::gl::GlslProgRef getShader() { return mSkinningShader; }
		const ci::gl::GlslProgRef getShader() const { return mSkinningShader; }
		
		void draw( std::shared_ptr<SkinnedMesh> skinnedMesh ) const override;
		void draw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh ) const override;
		void draw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" ) const override;
		//! Render the node names.
		void drawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera ) const override;
	private:
		GlRenderer();
		GlRenderer( ci::gl::GlslProgRef customShader );
		
		void drawSection( const ASkinnedMesh& section, std::function<void()> drawMesh ) const;
		//! Determines whether the node should be rendered as part of the skeleton.
		bool	isVisibleNode( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node ) const;
		//! Draw the visible nodes/bones of the skeleton by traversing recursively its node transformation hierarchy.
		void	drawRelative( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node, const std::shared_ptr<Node>& parent = nullptr ) const;
		//! Draw the visible nodes/bones of the skeleton by using its absolute bone positions.
		void	drawAbsolute( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node ) const;
		
		
		//! Optional argument for precomputed distance
		void drawConnected( const ci::Vec3f& nodePos, const ci::Vec3f& parentPos ) const;
		void drawJoint( const ci::Vec3f& nodePos ) const;
		void drawBone( const ci::Vec3f& start, const ci::Vec3f& end, float dist = -1.0f ) const;
		void drawSkeletonNode( const Node& node, Node::RenderMode mode = Node::RenderMode::CONNECTED ) const;
		void drawSkeletonNodeRelative( const Node& node, Node::RenderMode mode = Node::RenderMode::CONNECTED ) const;
		void drawLabel( const Node& node, const ci::CameraPersp& camera, const ci::Matrix44f& mv ) const;
		
		ci::gl::GlslProgRef mSkinningShader;
	};
}