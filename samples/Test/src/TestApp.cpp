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
	for( auto &animationPair : animations ) {
		auto &animation = animationPair.second;
		auto paramData = animation.getParameters();
		AnimationWithTarget target;
		target.target = animation.channels[0].target->name;
		target.animTransform = gltf::Animation::createTransformClip( paramData );
		animTransforms.emplace_back( move( target ) );
	}
	auto mesh = gltf::MeshLoader( file, &file->getMeshInfo( "Cylinder-mesh" ) );
	mTrimesh = ci::TriMesh::create( mesh, TriMesh::Format().boneIndex().boneWeight().positions().normals() );
	mDrawable = ci::TriMesh::create( *mTrimesh, TriMesh::Format().positions() );
	for( int i = 0; i < mTrimesh->getNumVertices(); i++ ) {
		auto boneIndex = mTrimesh->getBoneIndices<4>()[i];
		auto boneWeight = mTrimesh->getBoneWeights<4>()[i];
		cout << i << " - Indices: " << boneIndex << " Weight: " << boneWeight << endl;
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
	std::array<ci::mat4, 2> mats, offsets;
	for( int i = 0; i < 2; i++ ) {
		auto &inverseBindPose = mSkeleton->jointArray[i].inverseBindPose;
		if( i == 0 ) {
			mats[0] = animTransforms[0].animTransform.getLooped( time ).getTRS();
			offsets[0] = inverseBindPose * mats[0];
		}
		else {
			mats[1] = mats[0] * animTransforms[1].animTransform.getLooped( time ).getTRS();
			offsets[1] = inverseBindPose * mats[1];
		}
	}
	auto &bufferInd = mTrimesh->getBufferBoneIndices();
	auto &bufferWei = mTrimesh->getBufferBoneWeights();
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
