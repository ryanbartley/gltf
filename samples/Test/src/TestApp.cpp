#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "gltf.h"
#include "MeshLoader.h"
#include "Transformation.hpp"
#include "Animation.h"
#include "Skeleton.h"

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
		ci::mat4	modelMatrix;
		ci::gl::BatchRef batch;
	};
	std::vector<Renderable> mRenderables;
	
	void iterateNode(  const gltf::Node &node );
};

void TestApp::setup()
{
	auto glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
	auto filePath = loadAsset( ci::fs::path( "RiggedSimple" ) / "glTF" / "riggedSimple.gltf" );
	auto file = gltf::File::create( filePath );
	
	const auto &skin = file->getSkinInfo( "Armature_Cylinder-skin" );
	auto skeleton = skin.createSkeleton( file );
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 10.0f );
	mCam.lookAt( vec3( 0, 0, -5 ), vec3( 0 ) );
	mCamUi.setCamera( &mCam );
	gl::enableDepthRead();
	gl::enableDepthWrite();
}

void TestApp::iterateNode( const gltf::Node &node )
{
	if( node.hasChildren() ) {
		const auto &parent = node;
		
	}
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
		gl::setModelMatrix( rend.modelMatrix );
		rend.batch->draw();
	}
}

CINDER_APP( TestApp, RendererGl )
