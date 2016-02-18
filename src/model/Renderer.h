
#pragma once

#include <memory>
#include <mutex>
#include <map>

#include "cinder/Camera.h"
#include "cinder/gl/GlslProg.h"

#include "AMeshSection.h"
#include "StaticMesh.h"
#include "SkeletalTriMesh.h"
#include "SkeletalMesh.h"
#include "MorphedMesh.h"
#include "Skeleton.h"
#include "Node.h"

namespace model {
	
	typedef std::shared_ptr<struct Light> LightRef;
	struct Light {
		static LightRef create() { return LightRef( new Light{} ); }
		ci::vec3	position;
		ci::ColorA	diffuse;
		ci::ColorA	specular;
	private:
		Light();
	};
	
	enum MeshType {
		STATIC,
		SKELETAL,
		MORPHED
	};
	
	class Renderer {
	public:
		virtual ~Renderer() { }
		static Renderer& instance();
		
		static const LightRef& getLight() { return mLight; }
		
		static ci::gl::GlslProgRef getShader( MeshType mtype ) { return instance().mShaders.at( mtype ); }
		static void setShader( MeshType mtype, ci::gl::GlslProgRef shader ) { instance().mShaders[ mtype ] = shader; }
		
		static void		draw( const StaticMeshRef& mesh, int sectionId = -1 );
		static void		draw( const SkeletalTriMeshRef& mesh, int sectionId = -1 );
		static void		draw( const SkeletalMeshRef& mesh, int sectionId = -1 );
		static void		draw( const MorphedMeshRef& mesh, int sectionId = -1 );
		
		static void		draw( const SkeletonRef& skeleton, const std::string& rootNodeName = "" );
		
		static void drawSkeletonNode( const NodeRef& node );
		
		//! Render the node names.
		static void		drawLabels( SkeletonRef skeleton, const ci::CameraPersp& camera );
	protected:
		Renderer();
		Renderer( const Renderer& renderer );
		Renderer& operator=( const Renderer& );
								
		static void bindGlobalUniforms( const ci::gl::GlslProgRef& shader );
		static void bindSectionUniforms( const std::shared_ptr<ABatchSection>& section, const ci::gl::GlslProgRef& shader );
		
		template <class T>
		static void drawSections( std::vector<T> sections, const ci::gl::GlslProgRef& shader, int sectionId ) {
			int index = -1;
			for( auto& section : sections ) {
				if( sectionId >= 0 && (sectionId != ++index) )
					continue;
				
				auto drawMesh = [&] {
					bindSectionUniforms( section, shader );
					section->getBatch()->draw();
				};
				drawSection( *section.get(), drawMesh );
			}
		}
		
		static void drawSection( const AMeshSection& section, std::function<void()> drawMesh );
		//! Determines whether the node should be rendered as part of the skeleton.
		static bool	isVisibleNode( SkeletonRef skeleton, const NodeRef& node );
		//! Draw the visible nodes/bones of the skeleton by traversing recursively its node transformation hierarchy.
		static void	drawRelative( SkeletonRef skeleton, const NodeRef& node );
		//! Draw the visible nodes/bones of the skeleton by using its absolute bone positions.
		static void	drawAbsolute( SkeletonRef skeleton, const NodeRef& node );
		
		static void drawBone( const ci::vec3& start, const ci::vec3& end );
		
		static std::unique_ptr<Renderer> mInstance;
		static std::once_flag mOnceFlag;
		
		std::map< MeshType, ci::gl::GlslProgRef> mShaders;

		static LightRef mLight;
	};
}

