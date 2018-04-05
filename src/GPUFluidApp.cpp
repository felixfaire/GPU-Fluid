#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GPUFluidApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void GPUFluidApp::setup()
{
}

void GPUFluidApp::mouseDown( MouseEvent event )
{
}

void GPUFluidApp::update()
{
}

void GPUFluidApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( GPUFluidApp, RendererGl )
