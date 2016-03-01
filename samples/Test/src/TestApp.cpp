#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "GltfTestContainer.hpp"

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
};

void TestApp::setup()
{
	mGltf.setup( "duck/glTF/duck.gltf", "LOD3spShape-lib", "texture_file2", "" );
	mCamUi.setCamera( &mGltf.getCamera() );
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
	mGltf.draw();
}

CINDER_APP( TestApp, RendererGl )
