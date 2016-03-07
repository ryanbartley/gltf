#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "GltfTestContainer.hpp"
#include "MeshLoader.h"

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
	
	GltfTestContainer mGltf;
	CameraUi mCamUi;
	gl::BatchRef mBatch;
	CameraPersp mCam;
};

void TestApp::setup()
{
	auto file = gltf::File::create( loadAsset( ci::fs::path( "duck" ) / "glTF-MaterialsCommon" / "duck.gltf" ) );
	gltf::MeshLoader mesh( file, "LOD3spShape-lib" );
	mBatch = gl::Batch::create( mesh, gl::getStockShader( gl::ShaderDef().color().lambert() ) );
	
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
	mBatch->draw();
}

CINDER_APP( TestApp, RendererGl )
