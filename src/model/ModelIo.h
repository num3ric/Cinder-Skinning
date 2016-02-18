#pragma once

#include "cinder/Vector.h"
#include "cinder/Surface.h"
#include "cinder/GeomIo.h"
#include "cinder/Exception.h"
#include "cinder/CinderAssert.h"

#include <array>
#include <map>
#include <string>
#include <unordered_map>

#include "cinder/gl/gl.h"
#include "cinder/Color.h"

namespace cinder {
	
	class Material {
	public:
		Material();
		Material( const ColorA &aAmbient, const ColorA &aDiffuse = ColorA( 0, 0, 0, 1 ), const ColorA &aSpecular = ColorA( 0, 0, 0, 1 ),
				 float aShininess = 1.0f, const ColorA &aEmission = ColorA( 0, 0, 0, 1 ), GLenum aFace = GL_FRONT );
		
		void	setAmbient( const ColorA &aAmbient ) { mAmbient = aAmbient; }
		void	setDiffuse( const ColorA &aDiffuse ) { mDiffuse = aDiffuse; }
		void	setSpecular( const ColorA &aSpecular ) { mSpecular = aSpecular; }
		void	setShininess( float aShininess ) { mShininess = aShininess; }
		void	setEmission( const ColorA &aEmission ) { mEmission = aEmission; }
		void	setFace( GLenum aFace ) { mFace = aFace; }
		
		ColorA	getAmbient() const { return mAmbient; }
		ColorA	getDiffuse() const { return mDiffuse; }
		ColorA	getSpecular() const { return mSpecular; }
		float	getShininess() const { return mShininess; }
		ColorA	getEmission() const { return mEmission; }
		GLenum	getFace() const { return mFace; }
		
	protected:
		ColorA			mAmbient;
		ColorA			mDiffuse;
		ColorA			mSpecular;
		float			mShininess;
		ColorA			mEmission;
		GLenum			mFace;
	};
} // namesapce

namespace model {
	
class Skeleton;
typedef std::shared_ptr<Skeleton> SkeletonRef;

class Node;
class ModelTarget;
	


struct MaterialSource
{
	enum TextureType {
		DIFFUSE = 0x1,
		
		/** The texture is combined with the result of the specular
		 *  lighting equation.
		 */
		SPECULAR = 0x2,
		
		/** The texture is combined with the result of the ambient
		 *  lighting equation.
		 */
		AMBIENT = 0x3,
		
		/** The texture is added to the result of the lighting
		 *  calculation. It isn't influenced by incoming light.
		 */
		EMISSIVE = 0x4,
		
		/** The texture is a height map.
		 *
		 *  By convention, higher gray-scale values stand for
		 *  higher elevations from the base height.
		 */
		HEIGHT = 0x5,
		
		/** The texture is a (tangent space) normal-map.
		 *
		 *  Again, there are several conventions for tangent-space
		 *  normal maps. Assimp does (intentionally) not
		 *  distinguish here.
		 */
		NORMALS = 0x6,
		
		/** The texture defines the glossiness of the material.
		 *
		 *  The glossiness is in fact the exponent of the specular
		 *  (phong) lighting equation. Usually there is a conversion
		 *  function defined to map the linear color values in the
		 *  texture to a suitable exponent. Have fun.
		 */
		SHININESS = 0x7,
		
		/** The texture defines per-pixel opacity.
		 *
		 *  Usually 'white' means opaque and 'black' means
		 *  'transparency'. Or quite the opposite. Have fun.
		 */
		OPACITY = 0x8,
		
		/** Displacement texture
		 *
		 *  The exact purpose and format is application-dependent.
		 *  Higher color values stand for higher vertex displacements.
		 */
		DISPLACEMENT = 0x9,
		
		/** Lightmap texture (aka Ambient Occlusion)
		 *
		 *  Both 'Lightmaps' and dedicated 'ambient occlusion maps' are
		 *  covered by this material property. The texture contains a
		 *  scaling value for the final color value of a pixel. Its
		 *  intensity is not affected by incoming light.
		 */
		LIGHTMAP = 0xA,
		
		/** Reflection texture
		 *
		 * Contains the color of a perfect mirror reflection.
		 * Rarely used, almost never for real-time applications.
		 */
		REFLECTION = 0xB
	};
	
	static const std::array<TextureType, 11> sTextureTypes;
	
	MaterialSource()
//	: mTransparentColor( ci::Color::white() )
//	, mUseAlpha( false )
	: mTwoSided( false )
	{ }
	std::map<TextureType, std::shared_ptr<ci::Surface>> mSurfaces;
	
	GLenum			mWrapS, mWrapT;
	ci::Material	mMaterial;
//	ci::Color		mTransparentColor;
//	bool			mUseAlpha;
	bool			mTwoSided;
};

/*!
 * Ultimately associated with each mesh vertex, Weights
 * refer to bones and their weight contribution (i.e. how
 * much they affect the displacement of a vertex).
 */
class Weights {
public:
	Weights() : mActiveNumWeights( 0 ), mWeights() { }
	static const int NB_WEIGHTS = 4;
	void		addWeight( const std::shared_ptr<Node>& bone, float weight );
	float		getWeight( int index ) const	{ return mWeights.at( index ); }
	const Node*	getBone( int index ) const		{ return mBones.at( index ); }
	size_t		getNumActiveWeights() const		{ return mActiveNumWeights; }
private:
	size_t mActiveNumWeights;
	std::array<float, NB_WEIGHTS>		mWeights;
	std::array<const Node*, NB_WEIGHTS>	mBones;
};
	
class AnimInfo {
public:
	AnimInfo() { }
	AnimInfo( float duration, float ticksPerSecond, const std::string &name )
	: mDuration( duration ), mTicksPerSecond( ticksPerSecond ), mName( name ) { }
	float getDuration() const { return mDuration; }
	float getTicksPerSecond() const { return mTicksPerSecond; }
	const std::string& getName() const { return mName; }
private:
	float		mDuration;
	float		mTicksPerSecond;
	std::string	mName;
};
	
typedef std::shared_ptr<class Source> SourceRef;
typedef std::shared_ptr<class SectionSource> SectionSourceRef;

class Source {
public:
	virtual std::vector<SectionSourceRef>	getSectionSources() const { return mSectionSources; }
	virtual std::shared_ptr<Node>			getSkeletonRoot() const { return mRootNode; }
	virtual std::unordered_map<std::string, std::shared_ptr<Node>>	getSkeletonBones() const { return mBones; }
	const std::vector<AnimInfo>&			getAnimInfos() const { return mAnimInfos; }
protected:
	//! Information extracted (upon class instantiation) from assimp about each model section
	std::vector<SectionSourceRef>	mSectionSources;
	std::vector<AnimInfo> mAnimInfos;
	std::shared_ptr<Node> mRootNode;
	std::unordered_map<std::string, std::shared_ptr<Node>> mBones;
};

class SectionSource : public ci::geom::Source {
public:
	friend class AssimpLoader;
	virtual size_t				getNumIndices() const override;
	virtual size_t				getNumVertices() const override;
	virtual ci::geom::Primitive	getPrimitive() const override;
	virtual uint8_t				getAttribDims( ci::geom::Attrib attr ) const override;
	virtual void				loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const override;
	virtual SectionSource*		clone() const { return new SectionSource( *this ); }
	ci::geom::AttribSet			getAvailableAttribs() const override;

	const std::string&				getName() const { return mName; }
	const MaterialSource&			getMaterialSource() const { return mMaterialSource; }
	const ci::mat4&			getDefaultTransformation() const { return mDefaultTransformation; }
	const std::vector<glm::vec4>&	getBoneIndices() const { return mBoneIndices; }
	const std::vector<glm::vec4>&	getBoneWeights() const { return mBoneWeights; }
	const std::vector<Weights>&		getWeights() const { return mWeights; }
	size_t							getNumMorphTargets() const { return mMorphOffsets.size(); }
private:
	bool				hasAttrib( ci::geom::Attrib attr ) const;
	
	std::string							mName;
	std::vector<glm::vec3>				mPositions, mNormals, mTangents, mBitangents;
	std::vector<glm::vec2>				mTexCoords;
	std::vector<uint32_t>				mIndices;
	std::vector<ci::Colorf>				mColors;
	std::vector<glm::vec4>				mBoneWeights, mBoneIndices;
	std::vector<Weights>				mWeights;
	MaterialSource						mMaterialSource;
	ci::mat4							mDefaultTransformation;

	std::vector<std::vector<glm::vec3>> mMorphOffsets;
};
	
class ModelIoException : public ci::Exception
{
public:
	ModelIoException() { mMessage[0] = 0; }
	ModelIoException( const std::string &message ) throw();
	
	virtual const char * what() const throw() { return mMessage.c_str(); }
	
private:
	std::string	mMessage;
};

class LoadErrorException : public ModelIoException {
public:
	LoadErrorException() : ModelIoException() { }
	LoadErrorException( const std::string &message ) throw() : ModelIoException( "Load error: " + message ) { };
};

class ModelTargetException : public ModelIoException {
public:
	ModelTargetException() : ModelIoException() {}
	ModelTargetException( const std::string &message ) throw() : ModelIoException( "Target error: " + message ) { };
};

} //end namespace model
