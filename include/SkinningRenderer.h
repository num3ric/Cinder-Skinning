
#pragma once
#include <memory>
#include <mutex>

#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"

#include "ASkinnedMesh.h"
#include "Node.h"

namespace model {
	
	class SkinnedMesh;
	class SkinnedVboMesh;
	class Skeleton;
	
	class SkinningRenderer {
	public:
		virtual ~SkinningRenderer() { }
		static SkinningRenderer& instance();
		SkinningRenderer(const SkinningRenderer& that) = delete;
		SkinningRenderer& operator=(const SkinningRenderer&) = delete;
		
		ci::gl::GlslProgRef			getShader() { return mSkinningShader; }
		const ci::gl::GlslProgRef	getShader() const { return mSkinningShader; }
		
		static void		draw( std::shared_ptr<SkinnedMesh> skinnedMesh );
		static void		draw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh );
		static void		draw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" );
		//! Render the node names.
		static void		drawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera );
	private:
		SkinningRenderer();
		void	privateDraw( std::shared_ptr<SkinnedMesh> skinnedMesh ) const;
		void	privateDraw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh ) const;
		void	privateDraw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" ) const;
		//! Render the node names.
		void	privateDrawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera ) const;
		
		static std::unique_ptr<SkinningRenderer> mInstance;
		static std::once_flag mOnceFlag;
		
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