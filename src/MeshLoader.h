//
//  GLTFLoader.h
//  GLTFWork
//
//  Created by Ryan Bartley on 11/22/14.
//
//

#include "cinder/GeomIo.h"
#include "cinder/ObjLoader.h"
#include "cinder/CinderAssert.h"
#include "gltf.h"

namespace gltf {

class MeshLoader : public ci::geom::Source {
public:
	MeshLoader( const FileRef &gltfFile, const std::string &meshId );
	
	~MeshLoader() = default;
	
	virtual size_t		getNumVertices() const { return mNumVertices; }
	virtual size_t		getNumIndices() const { return mNumIndices; }
	virtual ci::geom::Primitive	getPrimitive() const { return mPrimitive; }
	virtual uint8_t		getAttribDims( ci::geom::Attrib attr ) const;
	virtual void		loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const;
	
	virtual ci::geom::AttribSet	getAvailableAttribs() const { return mAvailableAttribs; }
	
	virtual Source*		clone() const { return new MeshLoader( *this ); }
	
	struct MeshInstance {
		MeshInstance( Material material, uint32_t first, uint32_t count )
		: material( std::move( material ) ), first( first ), count( count ) {}
		Material material;
		uint32_t first;
		uint32_t count;
	};
	
	const std::vector<MeshInstance>& getMeshInstances() { return mMeshInstances; }
	
private:
	void scanMesh( Mesh &mesh );
	
	template<typename T>
	void copyIndices( std::vector<uint32_t> &indices, const T* data, uint32_t count ) const;
	
	FileRef				mFile;
	std::string			mMeshId;
	ci::geom::AttribSet mAvailableAttribs;
	size_t				mNumVertices, mNumIndices;
	ci::geom::Primitive mPrimitive;
	std::map<ci::geom::Attrib, const Accessor*> mAttribAccessors;
	std::vector<const Accessor*>	mIndexAccessors;
	std::vector<MeshInstance>		mMeshInstances;
	
};
	
inline MeshLoader::MeshLoader( const FileRef &gltfFile, const std::string &meshId )
: mFile( gltfFile ), mMeshId( meshId ), mNumIndices( 0 ), mNumVertices( 0 ),
	mPrimitive( ci::geom::Primitive::NUM_PRIMITIVES )
{
	bool primitiveSet = false;
	bool verticesSet = false;
	const auto &mesh = gltfFile->getMeshInfo( mMeshId );
	
	for( const auto &prim : mesh.primitives ) {
		// Primitive Mode should be the same throughout.
		if( ! primitiveSet ) {
			mPrimitive = File::convertToPrimitive( prim.primitive );
			primitiveSet = true;
		}
		else
			CI_ASSERT( mPrimitive == File::convertToPrimitive( prim.primitive ) );
		
		// Go through the attributes
		for( const auto &attribAccessors : prim.attributes ) {
			const auto &vertAccessor = gltfFile->getAccessorInfo( attribAccessors.accessor );
			// The number of vertices should also be the same for each primitive
			if( ! verticesSet ) {
				mNumVertices = vertAccessor.count;
				verticesSet = true;
			}
			else
				CI_ASSERT( mNumVertices == vertAccessor.count );
			
			auto material = gltfFile->getMaterialInfo( prim.material );
			if( ! prim.indices.empty() ) {
				const auto &index = gltfFile->getAccessorInfo( prim.indices );
				mIndexAccessors.emplace_back( &index );
				mMeshInstances.emplace_back( std::move( material ), mNumIndices, index.count );
				mNumIndices += index.count;
			}
			else {
				
			}
			
			auto emplaced = mAttribAccessors.emplace( attribAccessors.attrib, &vertAccessor );
			if( ! emplaced.second )
				CI_ASSERT( emplaced.first->second == &vertAccessor );
			mAvailableAttribs.insert( attribAccessors.attrib );
		}
	}
}

inline uint8_t MeshLoader::getAttribDims( ci::geom::Attrib attr ) const
{
	auto found = mAttribAccessors.find( attr );
	if( found != mAttribAccessors.end() )
		return File::getNumComponentsForType( found->second->type );
	else
		return 0;
}
	
template<typename T>
inline void MeshLoader::copyIndices( std::vector<uint32_t> &indices, const T *data, uint32_t count ) const
{
	auto sizeOffset = indices.size();
	indices.resize( sizeOffset + count );
	for( int i = 0; i < count; i++ ) {
		indices[sizeOffset+i] = (*data++);
	}
}

inline void MeshLoader::loadInto( ci::geom::Target *target, const ci::geom::AttribSet &requestedAttribs ) const
{
	for( auto & attrib : requestedAttribs ) {
		auto found = mAttribAccessors.find( attrib );
		if( found != mAttribAccessors.end() ) {
			auto accessor = found->second;
			auto dims = File::getNumComponentsForType( accessor->type );
			const auto &bufferView = mFile->getBufferViewInfo( accessor->bufferView );
			const auto &buffer = mFile->getBufferInfo( bufferView.buffer );
			auto accessorOffset = accessor->byteOffset;
			auto bufferViewOffset = bufferView.byteOffset;
			auto count = accessor->count;
			auto dataPtr = reinterpret_cast<float*>( static_cast<uint8_t*>( buffer.data->getData() ) + bufferViewOffset + accessorOffset );
			target->copyAttrib( found->first, dims,  0, dataPtr, count );
		}
	}
	if( ! mIndexAccessors.empty() ) {
		std::vector<uint32_t> indices;
		for( auto index : mIndexAccessors ) {
			auto byteLengthPerComponent = File::getNumBytesForComponentType( index->componentType );
			const auto &bufferView = mFile->getBufferViewInfo( index->bufferView );
			const auto &buffer = mFile->getBufferInfo( bufferView.buffer );
			auto dataPtr = reinterpret_cast<uint8_t*>( buffer.data->getData() ) + bufferView.byteOffset + index->byteOffset;
			
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

}