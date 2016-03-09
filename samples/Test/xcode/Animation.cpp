//
//  Animation.cpp
//  Test
//
//  Created by ryan bartley on 3/8/16.
//
//

#include "Animation.h"

using namespace std;

Clip::Clip( const gltf::FileRef &file, const gltf::Animation &animation )
{
	const auto &timeAccessor = file->getAccessorInfo( animation.timeAccessor );
	CI_ASSERT( timeAccessor.type == "SCALAR" );
	auto totalKeyFrames = timeAccessor.count;
	const auto &bufferView = file->getBufferViewInfo( timeAccessor.bufferView );
	const auto &buffer = file->getBufferInfo( bufferView.buffer );
	std::vector<float> times( totalKeyFrames );
	auto dataPtr = reinterpret_cast<uint8_t*>(buffer.data->getData()) + bufferView.byteOffset + timeAccessor.byteOffset;
	memcpy( times.data(), dataPtr, totalKeyFrames * sizeof( float ) );
	for( auto & param : animation.parameters ) {
		const auto &accessor = file->getAccessorInfo( param.accessor );
		auto numComponents = gltf::File::getNumComponentsForType( accessor.type );
		auto numBytesPerComponent = gltf::File::getNumBytesForComponentType( accessor.componentType );
		auto offset = mData.size();
		CI_ASSERT( totalKeyFrames == accessor.count );
		mData.resize( offset + ( numComponents * totalKeyFrames ) );
		const auto &bufferView = file->getBufferViewInfo( accessor.bufferView );
		const auto &buffer = file->getBufferInfo( bufferView.buffer );
		auto dataPtr = reinterpret_cast<uint8_t*>(buffer.data->getData()) + bufferView.byteOffset + accessor.byteOffset;
		memcpy( &mData[offset], dataPtr, accessor.count * numComponents * numBytesPerComponent );
		std::vector<Keyframe> keyFrames;
		int j = offset;
		for( int i = 0; i < totalKeyFrames; i++ ) {
			keyFrames.emplace_back( j, numComponents, times[i] );
			j += numComponents;
		}
		mKeyFrames.emplace( param.parameter, std::move( keyFrames ) );
	}
	for( auto &keyFrame : mKeyFrames ) {
		cout << "Parameter: " << keyFrame.first << endl;
		int i = 0;
		for( auto & frame : keyFrame.second ) {
			cout << "\tKeyframe: " << i++ << " At time: " << frame.getTime() << " - [";
			auto endIndex = frame.getIndex() + frame.getCount();
			for( int i = frame.getIndex(); i < endIndex; i++ ) {
				cout << " " << mData[i] << (i < endIndex - 1 ? ", " : " ]");
			}
			cout << endl;
		}
	}
}