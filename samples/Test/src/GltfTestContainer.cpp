//
//  GltfTestContainer.cpp
//  Test
//
//  Created by ryan bartley on 11/28/15.
//
//

#include "GltfTestContainer.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

void GltfTestContainer::setup( const std::string &gltfPath, const std::string &meshKey,
							  const std::string &textureName, const std::string &glslName )
{
//	mGltf = make_shared<gltf::Gltf>( loadAsset( gltfPath ) );
//	
//	mVbo = gltf::gl::getVboMeshFromMeshByName( *mGltf, meshKey );
//	mTrimesh = gltf::gl::getTriMeshFromMeshByName( *mGltf, meshKey );
//	
//	if( ! textureName.empty() ) {
//		mTexture = gltf::gl::getTextureByName( *mGltf, textureName );
//		mTexture->bind( 0 );
//	}
//	if( ! glslName.empty() ) {
//		mGlsl = gltf::gl::getGlslProgramFromMaterial( *mGltf, glslName );
//		mGlsl->uniform( "uTex0", 0 );
//	}
//	else
//		if( ! mTexture )
//			mGlsl = gl::getStockShader( gl::ShaderDef().lambert().color() );
//		else {
//			mGlsl = gl::getStockShader( gl::ShaderDef().texture( mTexture ).lambert() );
//		}
//	
//	mBatch = gl::Batch::create( mVbo, mGlsl );
//	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.01f, 1000.0f );
//	mCam.lookAt( vec3( 0, 0, -5 ), vec3( 0, 0, 0 ) );
}

void GltfTestContainer::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	mBatch->draw();
}