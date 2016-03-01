//#include "cinder/app/App.h"
//#include "cinder/app/RendererGl.h"
//#include "cinder/gl/gl.h"
//
//#include "cinder/gl/Shader.h"
//#include "cinder/gl/Vao.h"
//#include "GLTF.h"
//#include "GLTFLoader.h"
//
//#include "cinder/Arcball.h"
//#include "cinder/gl/Shader.h"
//#include "GltfTestContainer.hpp"
//
//using namespace ci;
//using namespace ci::app;
//using namespace std;
//
//class GLTFWorkApp : public App {
//  public:
//	void setup();
//	void mouseDown( MouseEvent event );
//	void mouseDrag( MouseEvent event );
//	void update();
//	void draw();
//	
//	
//	ci::CameraPersp		mCam;
//	Arcball				mArcball;
//	GltfTestContainer	mTestContainer;
//};
//
//void GLTFWorkApp::setup()
//{
//	
//	mCam.setPerspective( 60.0, getWindowAspectRatio(), .1, 1000 );
//	mCam.lookAt( vec3( 0, 0, 5 ), vec3( 0.0f ) );
//	
//	gl::enableDepthRead();
//	gl::enableDepthWrite();
//}
//
//void GLTFWorkApp::mouseDown( MouseEvent event )
//{
//}
//
//void GLTFWorkApp::mouseDrag( cinder::app::MouseEvent event )
//{
//	
//}
//
//void GLTFWorkApp::update()
//{
//}
//
//void GLTFWorkApp::draw()
//{
//	static float i = 0.0f;
//	// clear out the window with black
//	
//}
//
//CINDER_APP( GLTFWorkApp, RendererGl )
