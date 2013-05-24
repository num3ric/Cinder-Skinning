#pragma once

#include <memory>
#include <mutex>

namespace model {
	
	class SkinnedMesh;
	class SkinnedVboMesh;
	class Skeleton;
	
	class Renderer {
	public:
		virtual ~Renderer() { } 
		virtual	void draw( std::shared_ptr<SkinnedMesh> skinnedMesh ) const = 0;
		virtual void draw( std::shared_ptr<SkinnedVboMesh> skinnedVboMesh ) const = 0;
		virtual void draw( std::shared_ptr<Skeleton> skeleton, bool absolute = true, const std::string& name = "" ) const = 0;
		virtual void drawLabels( std::shared_ptr<Skeleton> skeleton, const ci::CameraPersp& camera ) const = 0;
	};
}