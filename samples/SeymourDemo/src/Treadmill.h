//
//  Treadmill.h
//  SeymourDemo
//
//  Created by Éric Renaud-Houde on 2013-07-20.
//
//

#pragma once
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"

class Treadmill
{
public:
	Treadmill();
	void draw( float animCycle );
private:
	ci::gl::TextureRef mTexture, mShadow;
	ci::gl::GlslProgRef mShader;
};