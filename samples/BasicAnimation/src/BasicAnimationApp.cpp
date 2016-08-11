#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "cinder/gltf/File.h"
#include "cinder/gltf/MeshLoader.h"

#include "cinder/Animation.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicAnimationApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
	
	gltf::FileRef		mFile;
	
	struct Object {
		gl::BatchRef	batch;
		ci::mat4		modelMatrix;
		TransformClip	transformClip;
	};
	std::vector<Object> mObjects;
	
	ci::CameraPersp mCam;
	ci::CameraUi	mCamUi;
};

void BasicAnimationApp::setup()
{
	mFile = gltf::File::create( loadAsset( "glTF/BoxAnimated.gltf" ) );
	auto &scene = mFile->getDefaultScene();
	for( auto node : scene.nodes ) {
		if( ! node->meshes.empty() ) {
			geom::SourceMods meshCombo;
			for( auto mesh : node->meshes ) {
				meshCombo &= gltf::MeshLoader( mesh );
			}
	
			Object object;
			object.batch = gl::Batch::create( meshCombo, gl::getStockShader( gl::ShaderDef().color() ) );
			object.modelMatrix = node->getHeirarchyTransform();
			object.transformClip = mFile->collectTransformClipFor( node );
			
			mObjects.emplace_back( move( object ) );
		}
	}
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.01, 1000.0 );
	mCam.lookAt( vec3( 0, 0, -5 ), vec3( 0 ) );
	mCamUi.setCamera( &mCam );
	mCamUi.connect( getWindow() );
}

void BasicAnimationApp::mouseDown( MouseEvent event )
{
}

void BasicAnimationApp::update()
{
}

void BasicAnimationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	auto time =  getElapsedSeconds();
	gl::setMatrices( mCam );
	for( auto &object : mObjects ) {
		ci::mat4 transform = object.modelMatrix;
		if( ! object.transformClip.empty() )
			transform *= object.transformClip.getMatrixLooped( time );
			
		gl::setModelMatrix( transform );
		object.batch->draw();
	}
}

CINDER_APP( BasicAnimationApp, RendererGl )
