#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "gltf.h"
#include "MeshLoader.h"
#include "Transformation.hpp"
#include "Animation.h"
#include "Skeleton.h"
#include "BoxAnimated.h"

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
	
	std::shared_ptr<BoxAnimated> mBoxAnimated;
	SkeletonRef mSkeleton;
	
	gl::BatchRef mBatch;
	CameraPersp mCam;
	CameraUi mCamUi;
	
	struct Renderable {
		std::string nodeName;
		ci::mat4	modelMatrix;
		ci::gl::BatchRef batch;
	};
	std::vector<Renderable> mRenderables;
	struct AnimationWithTarget {
		std::string target;
		Clip<Transform> animTransform;
	};
	std::vector<AnimationWithTarget> animTransforms;
};

void TestApp::setup()
{
	auto glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
	auto filePath = loadAsset( ci::fs::path( "RiggedSimple" ) / "glTF" / "riggedSimple.gltf" );
	auto file = gltf::File::create( filePath );
	
	const auto &skin = file->getSkinInfo( "Armature_Cylinder-skin" );
	mSkeleton = skin.createSkeleton();
	const auto &animations = file->getAnimations();
	for( auto &animationPair : animations ) {
		auto &animation = animationPair.second;
		auto paramData = animation.getParameters();
		AnimationWithTarget target;
		target.target = animation.channels[0].target->name;
		target.animTransform = gltf::Animation::createTransformClip( paramData );
		animTransforms.emplace_back( move( target ) );
	}

//	mBoxAnimated.reset( new BoxAnimated );
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 10000.0f );
	mCam.lookAt( vec3( 0, 0, -20 ), vec3( 0 ) );
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
	const std::array<ci::Colorf, 2> colors = { Colorf( 1, 0, 0 ), Colorf( 0, 1, 0 ) };
	gl::ScopedModelMatrix scopeModel;
	auto time = getElapsedFrames() / 60.0;
	for( int i = 0; i < mSkeleton->jointNames.size(); i++ ) {
		auto &local = mSkeleton->bindPose.localPoses[i];
		gl::multModelMatrix( local.getTRS() );
		auto &trans = animTransforms[i];
		auto currentTransform = trans.animTransform.getLooped( time );
		auto mat = currentTransform.getTRS();
		gl::multModelMatrix( mat );
		gl::color( colors[i] );
		gl::drawCube( vec3( 0, 2.5, 0 ), vec3( 1, 5, 1 ) );
	}
//	for( auto & rend : mRenderables ) {
//		gl::ScopedModelMatrix scopeModel;
//		gl::setModelMatrix( rend.modelMatrix );
//		rend.batch->draw();
//	}
//	mBoxAnimated->draw();
}

CINDER_APP( TestApp, RendererGl )
