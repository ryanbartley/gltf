//
//  GLTFLoader.cpp
//  GLTFWork
//
//  Created by Ryan Bartley on 11/22/14.
//
//

#include "MeshLoader.h"

using namespace std;

namespace cinder {
namespace gltf {
	
 MeshLoader::MeshLoader( const Mesh *mesh )
: mMesh( mesh ), mNumIndices( 0 ), mNumVertices( 0 ),
mPrimitive( ci::geom::Primitive::NUM_PRIMITIVES )
{
	bool primitiveSet = false;
	bool verticesSet = false;
	
	for( const auto &prim : mMesh->primitives ) {
		// Primitive Mode should be the same throughout.
		if( ! primitiveSet ) {
			mPrimitive = Mesh::convertToPrimitive( prim.primitive );
			primitiveSet = true;
		}
		else
			CI_ASSERT( mPrimitive == Mesh::convertToPrimitive( prim.primitive ) );
		
		// Go through the attributes
		for( const auto &attribAccessors : prim.attributes ) {
			const auto vertAccessor = attribAccessors.accessor;
			// The number of vertices should also be the same for each primitive
			if( ! verticesSet ) {
				mNumVertices = vertAccessor->count;
				verticesSet = true;
			}
			else
				CI_ASSERT( mNumVertices == vertAccessor->count );
			
			if( prim.indices != nullptr ) {
				auto count = prim.indices->count;
				mIndexAccessors.emplace_back( prim.indices );
				mMeshInstances.emplace_back( prim.material, mNumIndices, count );
				mNumIndices += count;
			}
			else {
				
			}
			
			auto emplaced = mAttribAccessors.emplace( attribAccessors.attrib, vertAccessor );
			if( ! emplaced.second )
				CI_ASSERT( emplaced.first->second == vertAccessor );
			mAvailableAttribs.insert( attribAccessors.attrib );
		}
	}
	verticesSet = true;
}

 uint8_t MeshLoader::getAttribDims( ci::geom::Attrib attr ) const
{
	auto found = mAttribAccessors.find( attr );
	if( found != mAttribAccessors.end() )
		return found->second->getNumComponents();
	else
		return 0;
}

template<typename T>
 void MeshLoader::copyIndices( std::vector<uint32_t> &indices, const T *data, uint32_t count ) const
{
	auto sizeOffset = indices.size();
	indices.resize( sizeOffset + count );
	for( int i = 0; i < count; i++ ) {
		indices[sizeOffset+i] = (*data++);
	}
}

 void MeshLoader::loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const
{
	for( auto & attrib : requestedAttribs ) {
		auto found = mAttribAccessors.find( attrib );
		if( found != mAttribAccessors.end() ) {
			auto accessor = found->second;
			auto dims = accessor->getNumComponents();
			auto count = accessor->count;
			auto dataPtr = reinterpret_cast<float*>(accessor->getDataPtr());
			target->copyAttrib( found->first, dims,  0, dataPtr, count );
		}
	}
	if( ! mIndexAccessors.empty() ) {
		std::vector<uint32_t> indices;
		for( auto index : mIndexAccessors ) {
			auto byteLengthPerComponent = index->getNumBytesForComponentType();
			auto dataPtr = reinterpret_cast<uint8_t*>(index->getDataPtr());
			
			if( byteLengthPerComponent == 1 )
				copyIndices( indices, dataPtr, index->count );
			else if( byteLengthPerComponent == 2 )
				copyIndices( indices, reinterpret_cast<uint16_t*>( dataPtr ), index->count );
			else if( byteLengthPerComponent == 4 )
				copyIndices( indices, reinterpret_cast<uint32_t*>( dataPtr ), index->count );
		}
		
		size_t totalNumIndices = indices.size();
		uint8_t bytesRequired = 0;
		if( totalNumIndices < std::numeric_limits<uint8_t>::max() )
			bytesRequired = sizeof( uint8_t );
		else if( totalNumIndices < std::numeric_limits<uint16_t>::max() )
			bytesRequired = sizeof( uint16_t );
		else if( totalNumIndices < std::numeric_limits<uint32_t>::max() )
			bytesRequired = sizeof( uint32_t );
		else
			CI_ASSERT_MSG( totalNumIndices < std::numeric_limits<uint32_t>::max(), "Can't exceed uint32_t max amount of indices." );
		
		target->copyIndices( mPrimitive, indices.data(), totalNumIndices,  bytesRequired );
	}
}
	
} // namespace gltf
} // namespace cinder