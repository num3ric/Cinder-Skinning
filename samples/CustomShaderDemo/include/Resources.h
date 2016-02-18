#pragma once
#include "cinder/CinderResources.h"

#define RES_STATIC_VERT         CINDER_RESOURCE( ../../../resources/, static_vert.glsl, 128, GLSL )
#define RES_SKINNING_VERT		CINDER_RESOURCE( ../../../resources/, skinning_vert.glsl, 128, GLSL )
#define RES_MORPH_VERT			CINDER_RESOURCE( ../../../resources/, morph_vert.glsl, 129, GLSL  )
#define RES_MODEL_FRAG			CINDER_RESOURCE( ../../../resources/, model_frag.glsl, 130, GLSL )

#define RES_CUSTOM_VERT				CINDER_RESOURCE( ../resources/, custom_vert.glsl, 131, GLSL )
#define RES_CUSTOM_FRAG				CINDER_RESOURCE( ../resources/, custom_frag.glsl, 132, GLSL )
#define RES_SEYMOUR					CINDER_RESOURCE( ../resources/, astroboy_walk.dae, 133, MESH )
