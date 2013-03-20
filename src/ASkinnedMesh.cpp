//
//  AbstractSkinnedMesh.cpp
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-06.
//
//

#include "ASkinnedMesh.h"

namespace model {

void ASkinnedMesh::draw()
{
	if( mHasDefaultTransformation ) {
		ci::gl::pushModelView();
		ci::gl::multModelView( mDefaultTransformation );
	}
	
	if ( mMatInfo.mTexture ) {
		mMatInfo.mTexture.enableAndBind();
	}
	mMatInfo.mMaterial.apply();
	
	// Culling
	if (  mMatInfo.mTwoSided ) {
		ci::gl::disable( GL_CULL_FACE );
	} else {
		ci::gl::enable( GL_CULL_FACE );
	}
	
	drawMesh();
	
	// Texture Binding
	if (  mMatInfo.mTexture ) {
		mMatInfo.mTexture.unbind();
	}
	
	if( mHasDefaultTransformation ) {
		ci::gl::popModelView();
	}
}

} //end namespace model
