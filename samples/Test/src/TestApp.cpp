//#include "cinder/app/App.h"
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
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	void loadFromFile( const fs::path &path, const string &nodeName );
	
	vector<pair<fs::path, string>> gltf;
	
	CameraPersp		mCam;
	CameraUi		mCamUi;
	
	SkeletonRef						mSkeleton;
	Skeleton::AnimRef				mSkeletonAnim;
	std::vector<TransformClip>		mSkeletonTransClip;
	
	shared_ptr<SkeletonRenderer>	mSkelRend;
	
	ci::mat4		modelMatrix, armatureMatrix;
	gl::BatchRef	mBatch, pose;
	gl::GlslProgRef mGlsl;
	std::vector<ci::mat4> localTransforms, offsetsSkel, offsetsRend;
	
	bool		pause = false, renderMesh = true, renderSkel = false, renderPose = false;
	int32_t		index = 0;
	uint32_t	frameNum = 0;
};

void TestApp::setup()
{
	mSkelRend.reset( new SkeletonRenderer() );
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), .01f, 10000.0f );
	mCam.lookAt( vec3( 0, 0, -5 ), vec3( 0 ) );
	mCam.setPivotDistance( 5 );
	mCamUi.setCamera( &mCam );
	mCamUi.connect( getWindow() );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	mGlsl = gl::GlslProg::create( gl::GlslProg::Format()
								 .vertex( loadAsset( "Cesium_Man0VS.glsl" ) )
								 .fragment( loadAsset( "Cesium_Man0FS.glsl" ) ) );
	
	gltf.emplace_back( fs::path( "monster" ) / "glTF" / "monster.gltf", string( "monster" ) );
	gltf.emplace_back( fs::path( "CesiumMan" ) / "glTF" / "Cesium_Man.gltf", string( "Cesium_Man" ) );
	gltf.emplace_back( fs::path( "brainsteam" ) / "glTF" / "brainsteam.gltf", string( "Figure_2_node" ) );
	
	auto &initial = gltf[0];
	loadFromFile( initial.first, initial.second );
}

void TestApp::loadFromFile( const fs::path &path, const std::string &nodeName )
{
	auto fileAsset = loadAsset( path );
	auto file = gltf::File::create( fileAsset );
	
	auto &node = file->getNodeInfo( nodeName );
	auto &meshes = node.meshes;
	
	
	const auto &skin = node.skin;
	mSkeleton = skin->createSkeleton();
	mSkeletonTransClip = file->createSkeletonTransformClip( mSkeleton );
	mSkeletonAnim = make_shared<Skeleton::Anim>( mSkeletonTransClip );
	
	geom::SourceMods geomCombo;
	ci::AxisAlignedBox aabb;
	for( auto mesh : meshes ) {
		auto geomSource = gltf::MeshLoader( mesh );
		aabb.include( mesh->getPositionAABB() );
		geomCombo &= geomSource;
	}
	
	mBatch = gl::Batch::create( geomCombo, mGlsl );
	pose = gl::Batch::create( geomCombo, gl::getStockShader( gl::ShaderDef() ) );
	
	modelMatrix = node.getHeirarchyTransform();
	
	if( nodeName == "monster" )
		modelMatrix = modelMatrix * (ci::translate( vec3( -1, 0, .3 ) ) * ci::scale( vec3( 0.001 ) ) );
	else if( nodeName == "Figure_2_node" )
		modelMatrix = mat4();
	// need to address the armature heirarchical transform
	armatureMatrix = skin->joints[0]->getHeirarchyTransform();
	
	aabb.transform( modelMatrix );
}

void TestApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
		case KeyEvent::KEY_SPACE: pause = !pause; return;
		case KeyEvent::KEY_RIGHT: index = (index + 1) % gltf.size(); break;
		case KeyEvent::KEY_LEFT: index = (index - 1) % gltf.size(); break;
		case KeyEvent::KEY_p: renderPose = !renderPose; return;
		case KeyEvent::KEY_m: renderMesh = !renderMesh; return;
		case KeyEvent::KEY_s: renderSkel = !renderSkel; return;
		case KeyEvent::KEY_r: mCam.lookAt( vec3( 0, 0, -10 ), vec3( 0 ) ); return;
		default: return;
	}
	
	const auto &next = gltf[index];
	loadFromFile( next.first, next.second );
}

void TestApp::update()
{
	if( ! pause )
		frameNum++;
	auto time = frameNum / 180.0;
	
	localTransforms.clear();
	for( auto &transClip : mSkeletonTransClip )
		localTransforms.emplace_back( transClip.getMatrixLooped( time ) );
	
	mSkeleton->calcGlobalMatrices( localTransforms, &offsetsSkel );
	mSkeleton->calcMatrixPaletteFromLocal( localTransforms, &offsetsRend );
}

void TestApp::draw()
{
	gl::clear();

	gl::setMatrices( mCam );
	
	if( ! renderPose ) {
		gl::ScopedModelMatrix scopeModel;
		gl::setModelMatrix( modelMatrix );
		if( renderMesh ) {
			auto &glsl = mBatch->getGlslProg();
			glsl->uniform( "uJointMat", offsetsRend.data(), offsetsRend.size() );
			mBatch->draw();
		}
		if( renderSkel )
			mSkelRend->draw( *mSkeleton, offsetsSkel );
	}
	else {
		if( renderMesh )
			pose->draw();
		if( renderSkel )
			mSkelRend->draw( *mSkeleton );
	}
}

CINDER_APP( TestApp, RendererGl )
