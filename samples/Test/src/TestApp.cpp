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
	auto file = gltf::File::create( loadAsset( ci::fs::path( "box" ) / "glTF" / "box.gltf" ) );
	file->getAccessorInfo( "" );
}

void TestApp::mouseDown( MouseEvent event )
{
}

void TestApp::mouseDrag( MouseEvent event )
{
}

void TestApp::update()
{
}

void TestApp::draw()
{
}

CINDER_APP( TestApp, RendererGl )
