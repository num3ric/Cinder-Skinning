
#pragma once

// note: this file will be removed at a later point

#include "cinder/app/App.h"

//#define NDEBUG //preprocessor definition

#if defined( CINDER_COCOA )
#define LOG_FUNCTION_CALL __PRETTY_FUNCTION__
#else
#define LOG_FUNCTION_CALL __FUNCTION__
#endif

#ifdef NDEBUG
#define LOG_CHECK false &&
#else
#define LOG_CHECK /*empty*/
#endif

#define LOG_M LOG_CHECK ci::app::console()
#define LOG_V LOG_M << LOG_FUNCTION_CALL << " | "
#define LOG_E LOG_V << __LINE__ << " | " << " *** ERROR *** : "