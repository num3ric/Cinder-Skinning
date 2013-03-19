//
//  AbstractSkinnedMesh.cpp
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-06.
//
//

#include "ASkinnedMesh.h"

void ASkinnedMesh::draw()
{
	if( mHasDefaultTransformation ) {
		gl::pushModelView();
		gl::multModelView( mDefaultTransformation );
	}
	
	if ( mMatInfo.mTexture ) {
		mMatInfo.mTexture.enableAndBind();
	}
	mMatInfo.mMaterial.apply();
	
	// Culling
	if (  mMatInfo.mTwoSided ) {
		gl::disable( GL_CULL_FACE );
	} else {
		gl::enable( GL_CULL_FACE );
	}
	
	if( mMatInfo.mUseAlpha ) {
		gl::disable( GL_CULL_FACE );
//		gl::enableAlphaBlending();
	} else {
//		gl::disableAlphaBlending();
	}
	
	drawMesh();
	
	// Texture Binding
	if (  mMatInfo.mTexture ) {
		mMatInfo.mTexture.unbind();
	}
	
	if( mHasDefaultTransformation ) {
		gl::popModelView();
	}
}
