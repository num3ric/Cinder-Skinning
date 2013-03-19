//
//  AbstractSkinnedMesh.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-06.
//
//

#pragma once

#include "Skeleton.h"
#include "ModelIo.h"

#include "cinder/gl/Material.h"
#include "cinder/gl/Texture.h"

#include <vector>

class ASkinnedMesh
{
public:
	bool	hasTexture() { return mMatInfo.mTexture; }
	bool	hasMaterial() { return mMatInfo.mHasMaterial; }
	bool	hasNormals() const { return mHasNormals; }
	void	setHasNormals( bool hasNormals ) { mHasNormals = hasNormals; }
	void	draw();
	
	void								appendBoneWeights( const BoneWeights& b ) { mBoneWeights.push_back( b ); }
	std::vector<BoneWeights>&			getBoneWeights() { return mBoneWeights; }
	const std::vector<BoneWeights>&		getBoneWeights() const { return mBoneWeights; }
	void								setBoneWeights( const std::vector<BoneWeights>& boneWeights ) { mBoneWeights = boneWeights; }
	
	void					setDefaultTransformation( const Matrix44f& transformation ) { mDefaultTransformation = transformation; mHasDefaultTransformation = true; }
	const Matrix44f&		getDefaultTranformation() const { return mDefaultTransformation; }
	
	void					setSkeleton( SkeletonRef skeleton ) { mSkeleton = skeleton; }
	SkeletonRef&			getSkeleton() { return mSkeleton; }
	const SkeletonRef&		getSkeleton() const { return mSkeleton; }
	
	MaterialInfo&			getMatInfo() { return mMatInfo; }
	const MaterialInfo&		getMatInfo() const { return mMatInfo; }
	void					setMatInfo( const MaterialInfo& matInfo ) { mMatInfo = matInfo; }
protected:
	ASkinnedMesh()
	: mIsAnimated( false )
	, mHasNormals( false )
	, mHasDefaultTransformation( false )
	{ };
	virtual void	drawMesh() = 0;
	virtual void	updateMesh( float time, bool enableSkinning = true ) = 0;
	
	bool		mIsAnimated;
	bool		mHasNormals;
	bool		mHasDefaultTransformation;
	
	SkeletonRef			mSkeleton;
	MaterialInfo		mMatInfo;
	Matrix44f			mDefaultTransformation;
	
	std::vector<BoneWeights>	mBoneWeights;
	
};