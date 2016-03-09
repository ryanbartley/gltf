//
//  Animation.hpp
//  Test
//
//  Created by ryan bartley on 3/8/16.
//
//

#pragma once

#include "gltf.h"

class Keyframe {
public:
	Keyframe( uint32_t index, uint8_t count, float time )
	: mIndex( index ), mCount( count ), mAtTime( time ) {}
	
	float	 getTime() { return mAtTime; }
	uint32_t getIndex() { return mIndex; }
	uint8_t	 getCount() { return mCount; }
	
private:
	uint32_t	mIndex;
	uint8_t		mCount;
	float		mAtTime;
};

class Clip {
public:
	Clip( const gltf::FileRef &file, const gltf::Animation &param );
	
	void updateGlobalTime( float globalTime, float elapsedSeconds );
	
private:
	
	std::map<std::string, std::vector<Keyframe>> mKeyFrames;
	std::vector<float>				mData;
};

