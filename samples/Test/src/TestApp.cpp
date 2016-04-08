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
	ci::TriMeshRef mTrimesh, mDrawable;
};

void TestApp::setup()
{
//	auto glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
	auto filePath = loadAsset( ci::fs::path( "CesiumMan" ) / "glTF" / "Cesium_Man.gltf" );
	auto file = gltf::File::create( filePath );
	
	const auto &skin = file->getSkinInfo( "Armature_Cesium_Man-skin" );
	mSkeleton = skin.createSkeleton();
	mSkeletonAnim = file->createSkeletonAnim( mSkeleton );
	
	auto mesh = gltf::MeshLoader( file, &file->getMeshInfo( "Cesium_Man-mesh" ) );
	auto glsl = gl::GlslProg::create( gl::GlslProg::Format()
									 .vertex( loadAsset( "Cesium_Man0VS.glsl" ) )
									 .fragment( loadAsset( "Cesium_Man0FS.glsl" ) ) );
	
	mBatch = gl::Batch::create( mesh, glsl );
//	mTrimesh = ci::TriMesh::create( mesh, TriMesh::Format().boneIndex().boneWeight().positions().normals() );
//	mDrawable = ci::TriMesh::create( *mTrimesh, TriMesh::Format().positions().colors( 3 ) );
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
	auto time = getElapsedFrames() / 60.0;
	std::vector<ci::mat4> localTransforms;
	
	mSkeletonAnim->getLoopedLocal( time, &localTransforms );
	std::vector<ci::mat4> offsets;
	mSkeleton->calcMatrixPaletteFromLocal( localTransforms, &offsets );
	
	gl::setMatrices( mCam );
	gl::ScopedModelMatrix scopeModel;
	
	auto &glsl = mBatch->getGlslProg();
	glsl->uniform( "uJointMat", offsets.data(), offsets.size() );
	mBatch->draw();
//	
//	for( int i = 0; i < mTrimesh->getNumVertices(); i++ ) {
//		auto &pos = mTrimesh->getPositions<3>()[i];
//		const auto &boneWeight = mTrimesh->getBoneWeights<4>()[i];
//		const auto &boneIndices = mTrimesh->getBoneIndices<4>()[i];
//		auto &writeablePos = mDrawable->getPositions<3>()[i];
//		mat4 ret;
//		// Weight normalization factor
//		float normfac = 1.0 / (boneWeight.x + boneWeight.y + boneWeight.z + boneWeight.w);
//		
//		// Weight1 * Bone1 + Weight2 * Bone2
//		ret = normfac * boneWeight.w * offsets[int(boneIndices.w)]
//		+ normfac * boneWeight.z * offsets[int(boneIndices.z)]
//		+ normfac * boneWeight.y * offsets[int(boneIndices.y)]
//		+ normfac * boneWeight.x * offsets[int(boneIndices.x)];
//		
//		writeablePos = vec3(ret * vec4(pos, 1.0));
//	}
//	
//	gl::draw( *mDrawable );
	
//	for( auto & rend : mRenderables ) {
//		gl::ScopedModelMatrix scopeModel;
//		gl::setModelMatrix( rend.modelMatrix );
//		rend.batch->draw();
//	}
//	mBoxAnimated->draw();
}

CINDER_APP( TestApp, RendererGl )
