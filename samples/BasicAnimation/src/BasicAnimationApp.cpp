#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "cinder/gltf/File.h"
#include "cinder/gltf/MeshLoader.h"

#include "cinder/Animation.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicAnimationApp : public App {
  public:
	void setup() override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
	
	void recurseScene( const gltf::Node *node );
	
	gltf::FileRef		mFile;
	
	struct Object {
		gl::BatchRef	batch;
		gl::Texture2dRef texture;
		ci::ColorA		color;
		ci::vec3		translate, scale;
		ci::quat		rotation;
		ci::mat4		modelMatrix;
		TransformClip	transformClip;
	};
	std::vector<Object> mObjects;
	
	ci::CameraPersp mCam;
	ci::CameraUi	mCamUi;
};

void BasicAnimationApp::setup()
{
	mFile = gltf::File::create( loadAsset( "testSceneForRyan/testSceneForRyan.gltf" ) );
	auto &scene = mFile->getDefaultScene();
	for( auto node : scene.nodes ) {
		recurseScene( node );
	}
	
	mCam.setPerspective( 60.0f, getWindowAspectRatio(), 0.01, 100000.0 );
	mCam.lookAt( vec3( 0, 0, -40 ), vec3( 0 ) );
	mCamUi.setCamera( &mCam );
	mCamUi.connect( getWindow() );
}

void BasicAnimationApp::recurseScene( const gltf::Node *node )
{
	if( ! node->meshes.empty() ) {
		Object object;
		geom::SourceMods meshCombo;
		for( auto mesh : node->meshes ) {
			meshCombo &= gltf::MeshLoader( mesh );
			auto &sources = mesh->primitives[0].material->sources;
			if( ! sources.empty() ) {
				if( sources[0].texture ) {
					auto image = sources[0].texture->image->getImage();
					object.texture = gl::Texture2d::create( image );
				}
				else {
					object.color = sources[0].color;
				}
			}
		}
		
		gl::GlslProgRef glsl;
		if( object.texture ) {
			glsl = gl::getStockShader( gl::ShaderDef().lambert().texture() );
		}
		else {
			glsl = gl::getStockShader( gl::ShaderDef().color().lambert() );
		}
		
		object.batch = gl::Batch::create( meshCombo, glsl );
		if( ! node->transformMatrix.empty() )
			object.modelMatrix = node->getHeirarchyTransform();
		else {
			ci::mat4 parentHeirarchy;
			if( auto parentNode = node->getParent() ) {
				parentHeirarchy = parentNode->getHeirarchyTransform();
			}
			object.modelMatrix = parentHeirarchy;
			object.translate = node->getTranslation();
			object.rotation = node->getRotation();
			object.scale = node->getScale();
		}
		object.transformClip = mFile->collectTransformClipFor( node );
		
		mObjects.emplace_back( move( object ) );
	}
	
	if( ! node->children.empty() ) {
		for( auto child : node->children ) {
			recurseScene( child );
		}
	}
}

void BasicAnimationApp::keyDown( KeyEvent event )
{
	
}

void BasicAnimationApp::update()
{
}

void BasicAnimationApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	auto time =  getElapsedSeconds() * 0.5;
	gl::setMatrices( mCam );
	gl::ScopedDepth scopDepth( true );
	for( auto &object : mObjects ) {
		ci::mat4 transform = object.modelMatrix;
		if( ! object.transformClip.empty() ) {
			auto translate = object.transformClip.getTranslationLooped( time );
			auto rotation = object.transformClip.getRotationLooped( time );
			auto scale = object.scale * object.transformClip.getScaleLooped( time );
			transform *= glm::translate( translate );
			transform *= glm::toMat4( rotation );
			transform *= glm::scale( scale );
		}
		auto ctx = gl::context();
		auto &texture = object.texture;
		if( texture ) {
			gl::color( 1, 1, 1 );
			ctx->pushTextureBinding( texture->getTarget(), texture->getId(), 0 );
		}
		else {
			gl::color( object.color );
		}
			
		gl::setModelMatrix( transform );
		object.batch->draw();
		
		if( texture )
			ctx->popTextureBinding( texture->getTarget(), 0 );
	}
}

CINDER_APP( BasicAnimationApp, RendererGl( RendererGl::Options().msaa( 16 ) ) )
