//
//  Treadmill.cpp
//  SeymourDemo
//
//  Created by Éric Renaud-Houde on 2013-07-20.
//
//

#include "Treadmill.h"

#include "cinder/app/App.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "Resources.h"

using namespace ci;

Treadmill::Treadmill()
{
	mTexture = gl::Texture::create( loadImage( app::getAssetPath("hazard_stripes.jpg") ) );
	mTexture->setWrapS(GL_REPEAT);
//	mTexture->setMagFilter(GL_LINEAR);
//	mTexture->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
	
	mShadow = gl::Texture::create( loadImage( app::getAssetPath("shadow.png") ) );
	
	try {
		mShader = gl::GlslProg::create( app::loadResource(RES_TMILL_VERT), app::loadResource(RES_TMILL_FRAG) );
	}
	catch( gl::GlslProgCompileExc &exc ) {
		app::console() << "Shader compile error: " << std::endl;
		app::console() << exc.what();
	}
	
	gl::enable( mTexture->getTarget() );
	gl::enable( mShadow->getTarget() );
}

void Treadmill::draw( float animCycle )
{
	mTexture->bind(0);
	mShadow->bind(1);
	
	mShader->bind();
	mShader->uniform("cycle", animCycle );
	mShader->uniform( "tex_diffuse", 0 );
	mShader->uniform( "tex_shadow", 1 );
	
	float l = 15.0f;
	float w = 5.0f;
	float t = 0.5f*(l/w);
	float y = -0.2f;
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glNormal3f(0,1,0);
	glMultiTexCoord2f(GL_TEXTURE0,  t, 0.0f); glMultiTexCoord2f(GL_TEXTURE1,  t, 0.0f);
	glVertex3f(-w, y,-l);
	glMultiTexCoord2f(GL_TEXTURE0, -t, 0.0f); glMultiTexCoord2f(GL_TEXTURE1, -t, 0.0f);
	glVertex3f(-w, y, l);
	glMultiTexCoord2f(GL_TEXTURE0, -t, 1.0f); glMultiTexCoord2f(GL_TEXTURE1, -t, 1.0f);
	glVertex3f( w, y, l);
	glMultiTexCoord2f(GL_TEXTURE0,  t, 1.0f); glMultiTexCoord2f(GL_TEXTURE1,  t, 1.0f);
	glVertex3f( w, y,-l);
    glEnd();
	glDisable(GL_TEXTURE_2D);
	mShader->unbind();
	
	mShadow->unbind(1);
	mTexture->unbind(0);
}