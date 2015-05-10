#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/gl/Shader.h"
#include "cinder/gl/Vao.h"
#include "GLTF.h"
#include "GLTFLoader.h"

#include "cinder/Arcball.h"
#include "cinder/gl/Shader.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GLTFWorkApp : public App {
  public:
	void setup();
	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );
	void update();
	void draw();
	
	std::shared_ptr<gltf::Gltf> mGltf;
	gl::VaoRef			mVao;
	ci::CameraPersp		mCam;
	ci::TriMeshRef		mTrimesh;
	ci::gl::VboMeshRef	mVbo;
	gl::TextureRef		mTexture;
	Arcball				mArcball;
	gl::GlslProgRef		mGlsl;
	gl::BatchRef		mBatch;
	gl::VboRef			mPositions, mNormals, mTexCoords, mIndices;
};

void GLTFWorkApp::setup()
{
	
//	mGlsl = gl::GlslProg::create( gl::GlslProg::Format()
//#if ! defined( CINDER_GL_ES )
//								 .vertex( loadAsset( "basicTexture.vert" ) )
//								 .fragment( loadAsset( "basicTexture.frag" ) )
//#else
//								 .vertex( loadAsset( "basicTexture_ios.vert" ) )
//								 .fragment( loadAsset( "basicTexture_ios.frag" ) )
//#endif
//								 .attrib( geom::Attrib::POSITION, "position" )
//								 .attrib( geom::Attrib::TEX_COORD_0, "texCoord" )
//								 .uniform( gl::UNIFORM_MODEL_VIEW_PROJECTION, "mvp" ) );
//	
	mGltf = make_shared<gltf::Gltf>( "duck.gltf" );
	auto loader = gltf::GltfLoader( *mGltf, "LOD3sp" );
	
//	mVbo = mGltf->getVboMeshFromMeshByName( "LOD3spShape-lib" );
//	mTrimesh = mGltf->getTriMeshFromMeshByName( "LOD3spShape-lib" );
	mTexture = mGltf->getTextureByName( "texture_file2" );
//	auto glsl = mGltf->getGlslProgramFromMaterial( "blinn3-fx" );
	
//	mGltf->getNodeInfo( "LOD3sp" );
//	mGltf->getTechniqueInfo( "technique1" );

	mBatch = gl::Batch::create( loader, gl::getStockShader( gl::ShaderDef().color().lambert().texture( mTexture ) ) );
	
	mCam.setPerspective( 60.0, getWindowAspectRatio(), .1, 1000 );
	mCam.lookAt( vec3( 0, 0, 5 ), vec3( 0.0f ) );
	
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
}

void GLTFWorkApp::mouseDown( MouseEvent event )
{
}

void GLTFWorkApp::mouseDrag( cinder::app::MouseEvent event )
{
	
}

void GLTFWorkApp::update()
{
}

void GLTFWorkApp::draw()
{
	static float i = 0.0f;
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
	
//	gl::ScopedVao scopeVao( mVao );
//	gl::ScopedGlslProg scopeGlsl( mGlsl );
	gl::ScopedTextureBind scopeTexture( mTexture, 0 );
//	gl::ScopedBuffer		scopeBuffer( mIndices );
	
	gl::setMatrices( mCam );
	i += .01f;
	gl::multModelMatrix( ci::toMat4( ci::angleAxis( i, vec3( 0, 1, 0 ) ) ) );
//	gl::setDefaultShaderVars();
	
//	mGlsl->uniform( "tex0", 0 );
	
	mBatch->draw();
//	gl::drawElements( GL_TRIANGLES, mTrimesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
}

CINDER_APP( GLTFWorkApp, RendererGl )
