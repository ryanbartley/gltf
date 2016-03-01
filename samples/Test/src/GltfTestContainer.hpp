//
//  GltfTestContainer.hpp
//  Test
//
//  Created by ryan bartley on 11/28/15.
//
//

#pragma once

#include "cinder/gl/gl.h"
#include "GLTF.h"

class GltfTestContainer {
public:
	GltfTestContainer() {}
	
	void setup( const std::string &gltfPath,
			    const std::string &meshKey,
			    const std::string &textureName,
			    const std::string &glslName );
	void draw();
	
	ci::CameraPersp&	getCamera() { return mCam; }
private:
	
	std::shared_ptr<gltf::Gltf> mGltf;
	ci::gl::GlslProgRef			mGlsl;
	ci::gl::BatchRef			mBatch;
	ci::TriMeshRef				mTrimesh;
	ci::gl::VboMeshRef			mVbo;
	ci::gl::TextureRef			mTexture;
	ci::CameraPersp				mCam;
};
