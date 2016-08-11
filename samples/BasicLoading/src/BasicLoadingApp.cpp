#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "cinder/gltf/File.h"
#include "cinder/gltf/MeshLoader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicLoadingApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	gltf::FileRef		mFile;
	gl::BatchRef		mBatch;
	gl::Texture2dRef	mTex;
	
	ci::CameraPersp mCam;
	ci::CameraUi	mCamUi;
};

void BasicLoadingApp::setup()
{
	mFile = gltf::File::create( loadAsset( "Duck/glTF/Duck.gltf" ) );
	
	auto loader = gltf::MeshLoader( &mFile->getMeshInfo( "LOD3spShape-lib" ) );
	mBatch = gl::Batch::create( loader, gl::getStockShader( gl::ShaderDef().color().texture() ) );
	
	auto image = mFile->getImageInfo("file2").getImage();
	mTex = gl::Texture2d::create( image, gl::Texture2d::Format().loadTopDown() );
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.01f, 1000.0f );
	mCam.lookAt( vec3( 0, 0, -5 ), vec3( 0 ) );
	mCamUi.setCamera( &mCam );
	mCamUi.connect( getWindow() );
}

void BasicLoadingApp::mouseDown( MouseEvent event )
{
}

void BasicLoadingApp::update()
{
}

void BasicLoadingApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	
	gl::ScopedDepth			scopeDepth( true );
	gl::ScopedTextureBind	scopeTex( mTex );
	
	mBatch->draw();
}

CINDER_APP( BasicLoadingApp, RendererGl )
