//
//  AbstractSkinnedMesh.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-06.
//
//

#pragma once

#include "ModelIo.h"

#include "cinder/gl/Material.h"
#include "cinder/gl/Texture.h"

#include <vector>

namespace model {

class AMeshSection
{
public:
	bool	hasTexture() { return mMatInfo.mTexture; }
	bool	hasMaterial() { return mMatInfo.mHasMaterial; }
	bool	hasNormals() const { return mHasNormals; }
	void	setHasNormals( bool hasNormals ) { mHasNormals = hasNormals; }
	
	void								appendBoneWeights( const BoneWeights& b ) { mBoneWeights.push_back( b ); }
	std::vector<BoneWeights>&			getBoneWeights() { return mBoneWeights; }
	const std::vector<BoneWeights>&		getBoneWeights() const { return mBoneWeights; }
	void								setBoneWeights( const std::vector<BoneWeights>& boneWeights ) { mBoneWeights = boneWeights; }
	
	void						setDefaultTransformation( const ci::Matrix44f& transformation ) { mDefaultTransformation = transformation; mHasDefaultTransformation = true; }
	const ci::Matrix44f&		getDefaultTranformation() const { return mDefaultTransformation; }
	bool						hasDefaultTransformation() const { return mHasDefaultTransformation; }
	
	void								setSkeleton( std::shared_ptr<Skeleton> skeleton ) { mSkeleton = skeleton; }
	std::shared_ptr<Skeleton>&			getSkeleton() { return mSkeleton; }
	const std::shared_ptr<Skeleton>&	getSkeleton() const { return mSkeleton; }
	bool								hasSkeleton() const { return mSkeleton != nullptr; }
	
	MaterialInfo&			getMatInfo() { return mMatInfo; }
	const MaterialInfo&		getMatInfo() const { return mMatInfo; }
	void					setMatInfo( const MaterialInfo& matInfo ) { mMatInfo = matInfo; }
	
	bool					isAnimated() { return mIsAnimated; }
protected:
	AMeshSection()
	: mIsAnimated( false )
	, mHasNormals( false )
	, mHasDefaultTransformation( false )
	{ };
	virtual void	updateMesh( bool enableSkinning = true ) = 0;
	
	bool		mIsAnimated;
	bool		mHasNormals;
	bool		mHasDefaultTransformation;
	
	std::shared_ptr<Skeleton>	mSkeleton;
	MaterialInfo				mMatInfo;
	ci::Matrix44f				mDefaultTransformation;
	
	std::vector<BoneWeights>	mBoneWeights;
	
};

} //end namespace model
