#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "gltf.h"
#include "MeshLoader.h"
#include "Transformation.hpp"
#include "AnimTemp.h"

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
			Renderable rend{node, ci::mat4(), batch};
			if( ! nodeInfo.transformMatrix.empty() ) {
				rend.modelMatrix = nodeInfo.getTransformMatrix();
			}
			else {
				Transform trans;
				trans.setTranslation( nodeInfo.getTranslation() );
				trans.setRotation( nodeInfo.getRotation() );
				trans.setScale( nodeInfo.getScale() );
				rend.modelMatrix = trans.getTRS();
			}
			
			mRenderables.emplace_back( std::move( rend ) );
		}
		
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
		gl::setModelMatrix( rend.modelMatrix );
		rend.batch->draw();
	}
}

CINDER_APP( TestApp, RendererGl )
