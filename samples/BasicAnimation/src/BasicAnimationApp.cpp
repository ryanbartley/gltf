#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "cinder/gltf/File.h"
#include "cinder/gltf/MeshLoader.h"
#include "cinder/simple/Scene.h"

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
	auto kicking = "VC/glTF/VC.gltf";
	mFile = gltf::File::create( loadAsset( kicking ) );
	mScene = make_shared<gltf::simple::Scene>( mFile, &mFile->getDefaultScene() );
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.01, 100000.0 );
	mCam.lookAt( vec3( 0, 0, -6.0 ), vec3( 0 ) );
	
	mCamUi.setCamera( &mCam );
	mCamUi.connect( getWindow() );
}

void BasicAnimationApp::keyDown( KeyEvent event )
{
	static uint32_t currentCamera = 0;
	bool chooseCamera = false;
	if( event.getCode() == KeyEvent::KEY_SPACE )
		mScene->toggleAnimation();
	else if( event.getCode() == KeyEvent::KEY_LEFT ) {
		int32_t camera = currentCamera;
		currentCamera = glm::clamp( --camera, 0, (int32_t)mScene->numCameras() - 1 );
		chooseCamera = true;
	}
	else if( event.getCode() == KeyEvent::KEY_RIGHT ) {
		int32_t camera = currentCamera;
		currentCamera = glm::clamp( ++camera, 0, (int32_t)mScene->numCameras() - 1 );
		chooseCamera = true;
	}
	
	if( chooseCamera )
		mScene->selectCamera( currentCamera );
}

void BasicAnimationApp::update()
{
	mScene->update( app::getElapsedSeconds() );
}

void BasicAnimationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	gl::setMatrices( mCam );
	
	gl::ScopedDepth scopeDepth( true );
	mScene->renderScene();
}

CINDER_APP( BasicAnimationApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
