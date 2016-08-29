#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "cinder/gltf/File.h"
#include "cinder/gltf/MeshLoader.h"
#include "cinder/gltf/SimpleScene.h"

#include "cinder/Animation.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicAnimationApp : public App {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	gltf::FileRef		mFile;
	shared_ptr<gltf::simple::Scene>	mScene;
	
	ci::CameraPersp mCam;
	ci::CameraUi	mCamUi;
};

void BasicAnimationApp::setup()
{
	mFile = gltf::File::create( loadAsset( "animationTest_01-1_c4d/animationTest_01-1_c4d.gltf" ) );
	mScene = make_shared<gltf::simple::Scene>( mFile, &mFile->getDefaultScene() );
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.01, 100000.0 );
	mCam.lookAt( vec3( 0, 0, -40 ), vec3( 0 ) );
	mCamUi.setCamera( &mCam );
	mCamUi.connect( getWindow() );
}

void BasicAnimationApp::keyDown( KeyEvent event )
{
	if( event.getCode() == KeyEvent::KEY_SPACE )
		mScene->toggleAnimation();
}

void BasicAnimationApp::update()
{
	mScene->update();
}

void BasicAnimationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatrices( mCam );
	gl::ScopedDepth scopeDepth( true );
	mScene->renderScene();
}

CINDER_APP( BasicAnimationApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
