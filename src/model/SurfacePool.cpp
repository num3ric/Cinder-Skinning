#include "SurfacePool.h"

#include "cinder/ImageIo.h"

using namespace ci;
using namespace model;

const std::shared_ptr<ci::Surface>& SurfacePool::loadSurface( const ci::fs::path& filepath )
{
	if(  mSurfaces.count( filepath ) ) {
		return mSurfaces.at( filepath );
	} else {
		std::shared_ptr<ci::Surface> texture( new ci::Surface( ci::loadImage( filepath ) ) );
		mSurfaces.emplace( filepath, texture );
		return mSurfaces.at( filepath );
	}
}