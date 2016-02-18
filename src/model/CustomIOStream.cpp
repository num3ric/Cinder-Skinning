////
////  CustomIOStream.cpp
////  SkeletonTest
////
////  Created by Éric Renaud-Houde on 2013-03-12.
////
////
//
//#include "CustomIOStream.h"
//
//#include <stdio.h>
//#include <sys/stat.h>
//
//#include "cinder/Utilities.h"
//
//namespace model {
//
//CustomIOStream::CustomIOStream( FILE *file )
//{
//	mStream = ci::IoStreamFile::create(file);
//}
//
//size_t CustomIOStream::Read( void* pvBuffer, size_t pSize, size_t pCount)
//{
//	return mStream->readDataAvailable( pvBuffer, pSize * pCount );
//}
//
//size_t CustomIOStream::Write( const void* pvBuffer, size_t pSize, size_t pCount)
//{
//	mStream->writeData( pvBuffer, pSize * pCount );
//	return pSize * pCount;
//}
//
//aiReturn CustomIOStream::Seek( size_t pOffset, aiOrigin pOrigin)
//{
//	try {
//		if( pOrigin == aiOrigin_SET ) {
//			mStream->seekRelative( (off_t) pOffset);
//		} else if( pOrigin == aiOrigin_END ) {
//			mStream->seekAbsolute( mStream->size() );
//			mStream->seekRelative( (off_t) pOffset );
//		} else if( pOrigin == aiOrigin_CUR ) {
//			mStream->seekRelative( (off_t) pOffset );
//		}
//	} catch(...) {
//		return aiReturn_FAILURE;
//	}
//	return aiReturn_SUCCESS;
//}
//
//size_t CustomIOStream::Tell() const
//{
//	return mStream->tell();
//}
//
//size_t CustomIOStream::FileSize() const
//{
//	if( mStream->getFILE() )
//		return mStream->size();
//	else
//		return 0;
//}
//
//void CustomIOStream::Flush()
//{
//	if( mStream->getFILE() )
//		fflush( mStream->getFILE() );
//}
//
//bool CustomIOSystem::Exists( const char *pFile) const
//{
//	struct stat buf;
//    return (stat(pFile, &buf) != -1);
//}
//
//char CustomIOSystem::getOsSeparator() const
//{
//	return ci::getPathSeparator();
//}
//
//Assimp::IOStream* CustomIOSystem::Open( const char *pFile, const char *pMode) 
//{
//	FILE *file = fopen( pFile, pMode );
//	if( file != nullptr ) {
//		return new CustomIOStream( fopen( pFile, pMode ) );
//	}
//	return nullptr;
//}
//
//void CustomIOSystem::Close( Assimp::IOStream* streamFile)
//{
//	delete streamFile;
//}
//
//} //end namespace model
