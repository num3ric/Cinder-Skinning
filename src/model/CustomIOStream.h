//
//  CustomIOStream.h
//  SkeletonTest
//
//  Created by Éric Renaud-Houde on 2013-03-12.
//
//

//#pragma once
//
//#include "cinder/Stream.h"
//
//#include "assimp/IOStream.hpp"
//#include "assimp/IOSystem.hpp"
//
//namespace model {
//
//class CustomIOStream : public Assimp::IOStream
//{
//public:
//	CustomIOStream( FILE *file );
//	~CustomIOStream(void) { }
//	size_t Read( void* pvBuffer, size_t pSize, size_t pCount) override;
//	size_t Write( const void* pvBuffer, size_t pSize, size_t pCount) override;
//	aiReturn Seek( size_t pOffset, aiOrigin pOrigin) override;
//	size_t Tell() const override;
//	size_t FileSize() const override;
//	void Flush() override;
//protected:
//	friend class MyIOSystem;
//	CustomIOStream(void) { };
//private:
//	ci::IoStreamFileRef mStream;
//};
//
//class CustomIOSystem : public Assimp::IOSystem
//{
//public:
//	CustomIOSystem() { }
//private:
//	~CustomIOSystem() { }
//	// Check whether a specific file exists
//	bool Exists( const char *pFile ) const override;
//	// Get the path delimiter character we'd like to see
//	char getOsSeparator() const override;
//	// ... and finally a method to open a custom stream
//	Assimp::IOStream* Open( const char *pFile, const char *pMode) override;
//	void Close( Assimp::IOStream* streamFile) override;
//};
//
//} //end namespace model
