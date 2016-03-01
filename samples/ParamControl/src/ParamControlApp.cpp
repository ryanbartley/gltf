#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ParamControlApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ParamControlApp::setup()
{
}

void ParamControlApp::mouseDown( MouseEvent event )
{
}

void ParamControlApp::update()
{
}

void ParamControlApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( ParamControlApp, RendererGl )
