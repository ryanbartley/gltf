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
		ci::mat4	modelMatrix;
		ci::gl::BatchRef batch;
	};
	Clip<ci::vec3> animTrans;
	Clip<ci::quat> animRot;
	
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
			Renderable rend{nodeInfo.name, ci::mat4(), batch};
			
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
	const auto &animations = file->getAnimations();
	for( const auto &animationKV : animations ) {
		auto &animation = animationKV.second;
		
		auto paramData = animation.getParameters( file );
		if( paramData[1].paramName == "rotation" ) {
			animRot = gltf::Animation::createRotationClip( paramData );
		}
		else if( paramData[1].paramName == "translation" ) {
			animTrans = gltf::Animation::createTranslationClip( paramData );
		}
	}
	
	cout << animRot << endl << animTrans << endl;
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 10.0f );
	mCam.lookAt( vec3( 0, 0, -5 ), vec3( 0 ) );
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
		if( rend.nodeName == "inner_box" ) {
			auto elapsedSeconds = getElapsedFrames() / 60.0;
			Transform trans;
			trans.setTranslation( animTrans.get( elapsedSeconds ) );
			trans.setRotation( animRot.get( elapsedSeconds ) );
			gl::multModelMatrix( trans.getTRS() );
		}
		rend.batch->draw();
	}
}

CINDER_APP( TestApp, RendererGl )
