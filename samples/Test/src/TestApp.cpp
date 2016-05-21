#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/CameraUi.h"

#include "gltf.h"
#include "MeshLoader.h"
#include "Animation.h"
#include "Skeleton.h"
#include "BoxAnimated.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct Object {
	
};


class TestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void mouseDrag( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override { pause = !pause; }
	void update() override;
	void draw() override;
	
	void setupDancingRobot();
	void drawDancingRobot();
	
	void setupMan();
	void drawMan();
	
	std::shared_ptr<BoxAnimated> mBoxAnimated;
	SkeletonRef mSkeleton;
	Skeleton::AnimRef mSkeletonAnim;
	
	std::shared_ptr<SkeletonRenderer> mSkelRend;
	
	gl::BatchRef mBatch;
	CameraPersp mCam;
	CameraUi mCamUi;
	bool pause = false;
	uint32_t frameNum = 0;
	
	struct Renderable {
		std::string nodeName;
		ci::mat4	modelMatrix;
		ci::gl::BatchRef batch;
	};
	std::vector<Renderable> mRenderables;
};

void TestApp::setup()
{
	//setupDancingRobot();
	setupMan();
	mSkelRend = make_shared<SkeletonRenderer>();
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 10000.0f );
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
//	drawDancingRobot();
	drawMan();
}

void TestApp::setupMan()
{
	auto filePath = loadAsset( ci::fs::path( "CesiumMan" ) / "glTF" / "Cesium_Man.gltf" );
	auto file = gltf::File::create( filePath );
	
	const auto &skin = file->getSkinInfo( "Armature_Cesium_Man-skin" );
	mSkeleton = skin.createSkeleton();
	mSkeletonAnim = file->createSkeletonAnim( mSkeleton );
	
	auto mesh0 = gltf::MeshLoader( &file->getMeshInfo( "Cesium_Man-mesh" ) );
	auto glsl = gl::GlslProg::create( gl::GlslProg::Format()
									 .vertex( loadAsset( "Cesium_Man0VS.glsl" ) )
									 .fragment( loadAsset( "Cesium_Man0FS.glsl" ) ) );
	
	mBatch = gl::Batch::create( mesh0, glsl );
}

void TestApp::drawMan()
{
	if( ! pause )
		frameNum++;
	auto time = frameNum / 60.0;
	std::vector<ci::mat4> localTransformsComb, localTransforms;
	
	mSkeletonAnim->getLoopedLocal( time, &localTransforms );
	std::vector<ci::mat4> offsetsSkel, offsetsRend;
	mSkeleton->calcGlobalMatrices( localTransforms, &offsetsSkel );
	mSkeleton->calcMatrixPaletteFromLocal( localTransforms, &offsetsRend );
	
	gl::setMatrices( mCam );
	gl::ScopedModelMatrix scopeModel;
	
	auto &glsl = mBatch->getGlslProg();
	glsl->uniform( "uJointMat", offsetsRend.data(), offsetsRend.size() );
	//	mBatch->draw();
	mSkelRend->draw( *mSkeleton, offsetsSkel );
}

void TestApp::setupDancingRobot()
{
	auto filePath = loadAsset( ci::fs::path( "brainsteam" ) / "glTF" / "brainsteam.gltf" );
	auto file = gltf::File::create( filePath );
	
	const auto &skin = file->getSkinInfo( "Poser_scene_root_Figure_2_node-skin" );
	mSkeleton = skin.createSkeleton();
	mSkeletonAnim = file->createSkeletonAnim( mSkeleton );
	
	auto mesh0 = gltf::MeshLoader( &file->getMeshInfo( "Figure_2_geometry-mesh" ) );
	auto mesh1 = gltf::MeshLoader( &file->getMeshInfo( "Figure_2_geometry-meshsplit_0" ) );
	auto mesh2 = gltf::MeshLoader( &file->getMeshInfo( "Figure_2_geometry-meshsplit_1" ) );
	auto glsl = gl::GlslProg::create( gl::GlslProg::Format()
									 .vertex( loadAsset( "Cesium_Man0VS.glsl" ) )
									 .fragment( loadAsset( "Cesium_Man0FS.glsl" ) ) );
	
	mBatch = gl::Batch::create( mesh0 & mesh1 & mesh2, glsl );
}

void TestApp::drawDancingRobot()
{
	if( ! pause )
		frameNum++;
	auto time = frameNum / 60.0;
	std::vector<ci::mat4> localTransformsComb, localTransforms;
	
	mSkeletonAnim->getLoopedLocal( time, &localTransforms );
	std::vector<ci::mat4> offsetsSkel, offsetsRend;
	mSkeleton->calcGlobalMatrices( localTransforms, &offsetsSkel );
	mSkeleton->calcMatrixPaletteFromLocal( localTransforms, &offsetsRend );
	
	gl::setMatrices( mCam );
	gl::ScopedModelMatrix scopeModel;
	
	auto &glsl = mBatch->getGlslProg();
	glsl->uniform( "uJointMat", offsetsRend.data(), offsetsRend.size() );
//	mBatch->draw();
	mSkelRend->draw( *mSkeleton, offsetsSkel );
	
}

CINDER_APP( TestApp, RendererGl )
