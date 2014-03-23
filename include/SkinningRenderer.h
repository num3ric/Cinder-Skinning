
#pragma once
#include <memory>
#include <mutex>

#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"

#include "AMeshSection.h"
#include "Node.h"

namespace model {
	
	class SkinnedMesh;
	class SkinnedVboMesh;
	
	class SkinningRenderer {
	public:
		virtual ~SkinningRenderer() { }
		static SkinningRenderer& instance();
		
		ci::gl::GlslProgRef			getShader() { return mSkinningShader; }
		const ci::gl::GlslProgRef	getShader() const { return mSkinningShader; }
		
		static void		draw( std::shared_ptr<SkinnedMesh> skinnedMesh );
		static void		draw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh );
		static void		draw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" );
		//! Render the node names.
		static void		drawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera );
	private:
		SkinningRenderer();
		SkinningRenderer(const SkinningRenderer& that);
		SkinningRenderer& operator=(const SkinningRenderer&);
		
		void	privateDraw( std::shared_ptr<SkinnedMesh> skinnedMesh ) const;
		void	privateDraw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh ) const;
		void	privateDraw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" ) const;
		//! Render the node names.
		void	privateDrawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera ) const;
		
		static std::unique_ptr<SkinningRenderer> mInstance;
		static std::once_flag mOnceFlag;
		
		void drawSection( const AMeshSection& section, std::function<void()> drawMesh ) const;
		//! Determines whether the node should be rendered as part of the skeleton.
		bool	isVisibleNode( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node ) const;
		//! Draw the visible nodes/bones of the skeleton by traversing recursively its node transformation hierarchy.
		void	drawRelative( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node, const std::shared_ptr<Node>& parent = nullptr ) const;
		//! Draw the visible nodes/bones of the skeleton by using its absolute bone positions.
		void	drawAbsolute( std::shared_ptr<Skeleton> skeleton, const std::shared_ptr<Node>& node ) const;
		
		
		void drawBone( const ci::Vec3f& start, const ci::Vec3f& end ) const;
		void drawJoint( const ci::Vec3f& nodePos ) const;

		void drawSkeletonNode( const NodeRef& node, Node::RenderMode mode = Node::RenderMode::CONNECTED ) const;
		void drawSkeletonNodeRelative( const NodeRef& node, Node::RenderMode mode = Node::RenderMode::CONNECTED ) const;
		void drawLabel( const NodeRef& node, const ci::CameraPersp& camera, const ci::Matrix44f& mv ) const;
		
		ci::gl::GlslProgRef mSkinningShader;
	};
}