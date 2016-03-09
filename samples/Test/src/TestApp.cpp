#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "gltf.h"
#include "MeshLoader.h"
#include "Transformation.hpp"
#include "Animation.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class TestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	CameraUi mCamUi;
	gl::BatchRef mBatch;
	CameraPersp mCam;
	
	struct Renderable {
		std::string nodeName;
		Transform	transform;
		ci::gl::BatchRef batch;
	};
	
	std::vector<Renderable> mRenderables;
};

void TestApp::setup()
{
	auto file = gltf::File::create( loadAsset( ci::fs::path( "boxAnimated" ) / "glTF" / "glTF.gltf" ) );
	const auto &defaultScene = file->getDefaultScene();
	for( auto & node : defaultScene.nodes ) {
		const auto &nodeInfo = file->getNodeInfo( node );
		if( ! nodeInfo.meshes.empty() ) {
			auto &meshKey = nodeInfo.meshes[0];
			gltf::MeshLoader mesh( file, meshKey );
			auto batch = gl::Batch::create( mesh, gl::getStockShader( gl::ShaderDef().color().lambert() ) );
			Transform trans;
			if( ! nodeInfo.transformMatrix.empty() ) {
				trans.setMatrix( nodeInfo.getTransformMatrix() );
			}
			else {
				trans.setTranslation( vec4( nodeInfo.getTranslation(), 1.0f ) );
				trans.setRotation( nodeInfo.getRotation() );
				trans.setScale( nodeInfo.getScale() );
				trans.checkUpdated();
			}
			cout << nodeInfo << endl;
			Renderable rend{node, std::move( trans ), batch};
			mRenderables.emplace_back( std::move( rend ) );
		}
		
	}
	std::vector<Clip> mClips;
	const auto &animations = file->getAnimations();
	for( auto &animation : animations ) {
		auto & anim = animation.second;
		mClips.emplace_back( file, anim );
//		for( auto & param : anim.parameters ) {
//			const auto &accessor = file->getAccessorInfo( param.accessor );
//			auto numComponents = gltf::File::getNumComponentsForType( accessor.type );
//			auto numBytesPerComponent = gltf::File::getNumBytesForComponentType( accessor.componentType );
//			std::vector<float> data( numComponents * accessor.count );
//			const auto &bufferView = file->getBufferViewInfo( accessor.bufferView );
//			const auto &buffer = file->getBufferInfo( bufferView.buffer );
//			auto dataPtr = reinterpret_cast<uint8_t*>(buffer.data->getData()) + bufferView.byteOffset + accessor.byteOffset;
//			memcpy( data.data(), dataPtr, accessor.count * numComponents * numBytesPerComponent );
//			cout << param.parameter << endl;
//			for( auto &d : data ) {
//				cout << "\t" << d << endl;
//			}
//		}
		
	}

	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 10.0f );
	mCam.lookAt( vec3( 0, 0, -3 ), vec3( 0 ) );
	mCamUi.setCamera( &mCam );
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void TestApp::mouseDown( MouseEvent event )
{
	mCamUi.mouseDown( event );
}

void TestApp::mouseDrag( MouseEvent event )
{
	mCamUi.mouseDrag( event );
}

void TestApp::update()
{
}

void TestApp::draw()
{
	gl::clear();
	gl::setMatrices( mCam );
	for( auto & rend : mRenderables ) {
		gl::ScopedModelMatrix scopeModel;
		gl::setModelMatrix( rend.transform.getModelMatrix() );
		rend.batch->draw();
	}
}

CINDER_APP( TestApp, RendererGl )
