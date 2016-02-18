#pragma once

#include "cinder/Surface.h"

#include <unordered_map>
#include <boost/noncopyable.hpp>


#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

namespace std
{
    template<> struct hash<ci::fs::path>
    {
        size_t operator()(const ci::fs::path& p) const
        {
            return ci::fs::hash_value(p);
        }
    };
}

namespace model {
	
	typedef std::shared_ptr<class SurfacePool> SurfacePoolRef;
	
	class SurfacePool : public boost::noncopyable {
	public:
		const std::shared_ptr<ci::Surface>& loadSurface( const ci::fs::path& filepath );
	private:
		std::unordered_map<ci::fs::path, std::shared_ptr<ci::Surface>> mSurfaces;
	};
	
} //end namespace model
