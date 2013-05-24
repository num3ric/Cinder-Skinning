#pragma once
#include "cinder/CinderResources.h"

//TODO: remove the need for always having these two to compile the block
#define RES_SKINNING_VERT			CINDER_RESOURCE( ../../../resources/, skinning_vert_normals.glsl, 128, GLSL  )
#define RES_SKINNING_FRAG			CINDER_RESOURCE( ../../../resources/, skinning_frag_normals.glsl, 129, GLSL  )

#define RES_SEYMOUR					CINDER_RESOURCE( ../resources/, astroboy_walk.dae, 130, DAE )
#define RES_CUSTOM_VERT				CINDER_RESOURCE( ../resources/, custom_vert.glsl, 131, GLSL )
#define RES_CUSTOM_FRAG				CINDER_RESOURCE( ../resources/, custom_frag.glsl, 132, GLSL )
