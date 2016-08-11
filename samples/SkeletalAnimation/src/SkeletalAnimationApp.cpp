#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "cinder/gltf/File.h"
#include "cinder/gltf/MeshLoader.h"
#include "cinder/Skeleton.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define USE_SKELETON_ANIM 0

class SkeletalAnimationApp : public App {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	void loadFromFile( const fs::path &path, const string &nodeName );
	
	vector<pair<fs::path, string>> gltf;
	
	CameraPersp		mCam;
	CameraUi		mCamUi;
	
	gl::BatchRef	mBatch, mPose;
	gl::GlslProgRef mGlsl;
	
	mat4			mModelMatrix;
	
	SkeletonRef						mSkeleton;
	Skeleton::AnimRef				mSkeletonAnim;
	std::vector<TransformClip>		mSkeletonTransClip;
	std::vector<ci::mat4>			localTransforms, offsetsSkel, offsetsRend;
	
	shared_ptr<SkeletonRenderer>	mSkelRend;
	
	bool		mPause = false, mRenderMesh = true,
				mRenderSkel = false, mRenderPose = false;
	int32_t		mGltfIndex = 0;
};

void SkeletalAnimationApp::setup()
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
								 .vertex( loadAsset( "skeletal.vert" ) )
								 .fragment( loadAsset( "skeletal.frag" ) ) );
	
	gltf.emplace_back( fs::path( "monster" ) / "glTF" / "monster.gltf", "monster" );
	gltf.emplace_back( fs::path( "CesiumMan" ) / "glTF" / "Cesium_Man.gltf", "Cesium_Man" );
	gltf.emplace_back( fs::path( "brainsteam" ) / "glTF" / "brainsteam.gltf", "Figure_2_node" );
	
	auto &initial = gltf[0];
	loadFromFile( initial.first, initial.second );
}

void SkeletalAnimationApp::loadFromFile( const fs::path &path, const std::string &nodeName )
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
	mPose = gl::Batch::create( geomCombo, gl::getStockShader( gl::ShaderDef() ) );
	
	mModelMatrix = node.getHeirarchyTransform();
	
	if( nodeName == "monster" )
		mModelMatrix = mModelMatrix * (ci::translate( vec3( -1.0f, 0.0f, 0.3f ) ) *
									   ci::scale( vec3( 0.001f ) ) );
	else if( nodeName == "Figure_2_node" )
		mModelMatrix = mat4();
}

void SkeletalAnimationApp::keyDown( KeyEvent event )
{
	switch ( event.getCode() ) {
		case KeyEvent::KEY_SPACE: mPause = !mPause; return;
		case KeyEvent::KEY_RIGHT: mGltfIndex = (mGltfIndex + 1) % gltf.size(); break;
		case KeyEvent::KEY_LEFT: mGltfIndex = (mGltfIndex - 1) % gltf.size(); break;
		case KeyEvent::KEY_p: mRenderPose = !mRenderPose; return;
		case KeyEvent::KEY_m: mRenderMesh = !mRenderMesh; return;
		case KeyEvent::KEY_s: mRenderSkel = !mRenderSkel; return;
		case KeyEvent::KEY_r: mCam.lookAt( vec3( 0, 0, -10 ), vec3( 0 ) ); return;
		default: return;
	}
	
	const auto &next = gltf[mGltfIndex];
	loadFromFile( next.first, next.second );
}

void SkeletalAnimationApp::update()
{
	localTransforms.clear();
#if USE_SKELETON_ANIM
	mSkeletonAnim->getLoopedLocal( getElapsedSeconds(), &localTransforms );
#else
	for( auto &transClip : mSkeletonTransClip )
		localTransforms.emplace_back( transClip.getMatrixLooped( getElapsedSeconds() ) );
#endif
	
	mSkeleton->calcGlobalMatrices( localTransforms, &offsetsSkel );
	mSkeleton->calcMatrixPaletteFromLocal( localTransforms, &offsetsRend );
}

void SkeletalAnimationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	
	gl::setMatrices( mCam );
	
	gl::ScopedModelMatrix scopeModel;
	gl::setModelMatrix( mModelMatrix );
	
	if( ! mRenderPose ) {
		if( mRenderMesh ) {
			auto &glsl = mBatch->getGlslProg();
			glsl->uniform( "uJointMat", offsetsRend.data(), static_cast<int>( offsetsRend.size() ) );
			mBatch->draw();
		}
		if( mRenderSkel )
			mSkelRend->draw( *mSkeleton, offsetsSkel );
	}
	else {
		if( mRenderMesh )
			mPose->draw();
		if( mRenderSkel )
			mSkelRend->draw( *mSkeleton );
	}
}

CINDER_APP( SkeletalAnimationApp, RendererGl )
