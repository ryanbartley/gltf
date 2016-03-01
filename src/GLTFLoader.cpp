//
//  GLTFLoader.cpp
//  GLTFWork
//
//  Created by Ryan Bartley on 11/22/14.
//
//

#include "GLTFLoader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace gltf {
	
GltfLoader::GltfLoader( const gltf::Gltf &gltf, const std::string &nodeId )
{
	auto node = gltf.getNodeInfo( nodeId );
	
	for ( auto & meshId : node.meshes ) {
		auto meshInfo = gltf.getMeshInfo( meshId );
		Mesh mesh;
		
		cout << meshInfo.name << endl;
		
		if( ! meshInfo.primitives[0].indices.empty() ) {
			auto indexAccessor = gltf.getAccessorInfo( meshInfo.primitives[0].indices );
			auto indexBufferView = gltf.getBufferViewInfo( indexAccessor.bufferView );
			auto indexBuffer = gltf.getBufferInfo( indexBufferView.buffer );
			
			mesh.mNumIndices = indexAccessor.count;
			mesh.mBytesPerIndex = Gltf::getNumBytesForComponentType( indexAccessor.componentType );
			mesh.mIndices.resize( mesh.mNumIndices * mesh.mBytesPerIndex );
			uint8_t * data = (uint8_t*)indexBuffer.data->getData();
			memcpy(mesh.mIndices.data(), &data[indexBufferView.byteOffset + indexAccessor.byteOffset], mesh.mNumIndices * mesh.mBytesPerIndex );
		}
		
		for( auto & attribute : meshInfo.primitives[0].attributes ) {
			mAvailableAttribs.insert( attribute.second );
		
			auto attribAccessor = gltf.getAccessorInfo( attribute.first );
			auto attribBufferView = gltf.getBufferViewInfo( attribAccessor.bufferView );
			auto attribBuffer = gltf.getBufferInfo( attribBufferView.buffer );
			
			uint32_t numComponents = gltf.getNumComponentsForType( attribAccessor.type );
			auto numBytesComponent = gltf.getNumBytesForComponentType( attribAccessor.componentType );
			
			std::vector<uint8_t> attributeBuffer( attribAccessor.count * numComponents * numBytesComponent );
			
			uint8_t * data = (uint8_t*)attribBuffer.data->getData();
			
			memcpy( attributeBuffer.data(),
				   &data[attribBufferView.byteOffset + attribAccessor.byteOffset],
				   attribAccessor.count * numComponents * numBytesComponent);
			auto foundAttrib = mAttribVerticesSize.find( attribute.second );
			if( foundAttrib != mAttribVerticesSize.end() ) {
				foundAttrib->second += (attribAccessor.count * numComponents * numBytesComponent);
			}
			else {
				mAttribVerticesSize.insert( { attribute.second, attribAccessor.count * numComponents * numBytesComponent } );
			}
			mesh.mBufferLayout.append( attribute.second, numComponents, attribAccessor.byteStride, 0 );
			
			mesh.mNumVertices = attribAccessor.count;
			
			mesh.mAttributes.insert( make_pair( attribute.second, std::move( attributeBuffer ) ) );
		}
		mesh.mPrimitive = Gltf::convertToPrimitive( meshInfo.primitives[0].primitive );
		mMeshes.push_back( std::move( mesh ) );
	}
}
	
GltfLoader::~GltfLoader()
{
	
}
	
void GltfLoader::loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const
{
	Mesh masterMesh;
	std::vector<uint32_t> masterIndices;
	geom::Primitive primitive;
	
	for( auto & mesh : mMeshes ) {
		
		masterIndices.resize( masterIndices.size() + mesh.mNumIndices );
		for( int i = masterMesh.mNumIndices, j = 0; i < masterIndices.size(); i++, j++) {
			uint32_t index;
			if( mesh.mBytesPerIndex == 2 )
				index = *(((uint16_t*)mesh.mIndices.data()) + j);
			else if( mesh.mBytesPerIndex == 4 )
				index = *(((uint32_t*)mesh.mIndices.data()) + j);
			masterIndices[i] = index + masterMesh.mNumVertices;
		}
		
		for( auto & attrib : mesh.mAttributes ) {
			auto foundMasterAttrib = masterMesh.mAttributes.find( attrib.first );
			auto & verticesMesh = attrib.second;
			if( foundMasterAttrib != masterMesh.mAttributes.end() ) {
				
				auto & verticesMaster = foundMasterAttrib->second;
				auto originalSize = verticesMaster.size();
				auto size = verticesMesh.size();
				
				verticesMaster.resize( verticesMaster.size() + size );
				memcpy( &verticesMaster.data()[originalSize],
					   verticesMesh.data(), verticesMesh.size() );
			}
			else {
				std::vector<uint8_t> verticesMaster( verticesMesh.size() );
				memcpy( verticesMaster.data(), verticesMesh.data(), verticesMesh.size() );
				masterMesh.mAttributes.insert( make_pair( attrib.first, std::move( verticesMaster ) ) );
			}
		}
		
		masterMesh.mNumIndices += mesh.mNumIndices;
		masterMesh.mNumVertices += mesh.mNumVertices;
	}
	
	masterMesh.mBufferLayout = mMeshes[0].mBufferLayout;
	int k = 0;
	for( auto & attrib : masterMesh.mAttributes ) {
		cout << k++ << endl;
		if( requestedAttribs.count( attrib.first ) ) {
			auto attribInfo = masterMesh.mBufferLayout.getAttribInfo( attrib.first );
			auto size = attrib.second.size() / 4;
			target->copyAttrib( attrib.first,
							   attribInfo.getDims(),
							   attribInfo.getStride(),
							   (float*)attrib.second.data(),
							   size / attribInfo.getDims() );
		}
	}
	if( ! masterIndices.empty() ) {
		target->copyIndices( primitive, masterIndices.data(), masterIndices.size(), 4 );
	}
}
	
uint8_t	GltfLoader::getAttribDims( ci::geom::Attrib attr ) const
{
	for( auto & mesh : mMeshes ) {
		return mesh.mBufferLayout.getAttribDims( attr );
	}
	return 0;
}
	
size_t GltfLoader::getNumVertices() const
{
	size_t numVertices = 0;
	for( auto & mesh : mMeshes ) {
		numVertices += mesh.mNumVertices;
	}
	return numVertices;
}
	
size_t GltfLoader::getNumIndices() const
{
	size_t numIndices = 0;
	for( auto & mesh : mMeshes ) {
		numIndices += mesh.mNumIndices;
	}
	return numIndices;
}
	
ci::geom::Primitive GltfLoader::getPrimitive() const
{
	return ci::geom::Primitive::TRIANGLES;
}
	
}