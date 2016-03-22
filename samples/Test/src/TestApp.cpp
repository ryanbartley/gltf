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
	Skeleton::AnimRef mSkeletonAnim;
	
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
	ci::TriMeshRef mTrimesh, mDrawable;
};

void TestApp::setup()
{
	auto glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
	auto filePath = loadAsset( ci::fs::path( "RiggedSimple" ) / "glTF" / "riggedSimple.gltf" );
	auto file = gltf::File::create( filePath );
	
	const auto &skin = file->getSkinInfo( "Armature_Cylinder-skin" );
	mSkeleton = skin.createSkeleton();
	const auto &animations = file->getAnimations();
	std::vector<Clip<Transform>> skeletonAnims;
	skeletonAnims.reserve( mSkeleton->getNumJoints() );
	for( auto &boneName : mSkeleton->getJointNames() ) {
		auto found = std::find_if( animations.begin(), animations.end(),
		[boneName]( const std::pair<std::string, gltf::Animation> &animation ){
			return animation.second.target == boneName;
		});
		if( found != animations.end() ) {
			auto params = found->second.getParameters();
			skeletonAnims.emplace_back( gltf::Animation::createTransformClip( params ) );
		}
	}
	mSkeletonAnim.reset( new Skeleton::Anim( move( skeletonAnims ) ) );
	auto mesh = gltf::MeshLoader( file, &file->getMeshInfo( "Cylinder-mesh" ) );
	mTrimesh = ci::TriMesh::create( mesh, TriMesh::Format().boneIndex().boneWeight().positions().normals() );
	mDrawable = ci::TriMesh::create( *mTrimesh, TriMesh::Format().positions().colors( 3 ) );
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
	std::vector<ci::mat4> offsets;
	
	mSkeletonAnim->getLooped( time, offsets );
	
	for( int i = 0; i < mTrimesh->getNumVertices(); i++ ) {
		auto &pos = mTrimesh->getPositions<3>()[i];
		const auto &boneWeight = mTrimesh->getBoneWeights<4>()[i];
		const auto &boneIndices = mTrimesh->getBoneIndices<4>()[i];
		auto &writeablePos = mDrawable->getPositions<3>()[i];
		mat4 ret;
		// Weight normalization factor
		float normfac = 1.0 / (boneWeight.x + boneWeight.y);
		
		// Weight1 * Bone1 + Weight2 * Bone2
		ret = normfac * boneWeight.y * offsets[int(boneIndices.y)]
		    + normfac * boneWeight.x * offsets[int(boneIndices.x)];
		
		writeablePos = vec3(ret * vec4(pos, 1.0));
	}
	
	gl::draw( *mDrawable );
	
//	for( auto & rend : mRenderables ) {
//		gl::ScopedModelMatrix scopeModel;
//		gl::setModelMatrix( rend.modelMatrix );
//		rend.batch->draw();
//	}
//	mBoxAnimated->draw();
}

CINDER_APP( TestApp, RendererGl )
