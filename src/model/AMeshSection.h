#pragma once

#include "ModelIo.h"
#include "cinder/gl/Texture.h"

#include "cinder/gl/VboMesh.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/GlslProg.h"

#include <vector>
#include <string>

namespace model {
	
class Skeleton;

class AMeshSection
{
public:
	const glm::mat4&	getDefaultTranformation() const { return *mDefaultTransformation; }
	bool					hasDefaultTransformation() const { return mDefaultTransformation != nullptr; }
	const ci::Material&		getMaterial() const { return mMaterial; }
	ci::gl::Texture2dRef	getTexture() const { return mTexture; }
	const std::string&		getName() const { return mName; }
protected:
	AMeshSection( const SectionSourceRef& source )
	: mName( source->getName() )
	, mMaterial( source->getMaterialSource().mMaterial )
	, mDefaultTransformation( new glm::mat4( source->getDefaultTransformation() ) )
	{
		auto textureSurfaces = source->getMaterialSource().mSurfaces;
		if( textureSurfaces.count( model::MaterialSource::TextureType::DIFFUSE ) ) {
			
			mTexture = ci::gl::Texture2d::create( *textureSurfaces.at( model::MaterialSource::TextureType::DIFFUSE ),
												  ci::gl::Texture2d::Format().loadTopDown() );
		}
	}
	virtual ~AMeshSection() { };
	
	std::string					mName;
	ci::Material				mMaterial;
	ci::gl::Texture2dRef		mTexture;
	std::unique_ptr<glm::mat4>	mDefaultTransformation;
};


typedef std::shared_ptr< class ABatchSection> ABatchSectionRef;

class ABatchSection : public AMeshSection {
public:
	ABatchSection( const SectionSourceRef& source, ci::gl::GlslProgRef shader, ci::gl::Batch::AttributeMapping mapping = ci::gl::Batch::AttributeMapping() )
	: AMeshSection( source )
	, mBatch( ci::gl::Batch::create( ci::gl::VboMesh::create( *source ), shader, mapping ) )
	{
	}
	virtual ~ABatchSection() { };
	
	ci::gl::BatchRef	getBatch() const { return mBatch; }
protected:
	ci::gl::BatchRef	mBatch;
};

} //end namespace model
