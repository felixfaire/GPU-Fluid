#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "GPUFluid.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GPUFluidApp : public App {
  public:
	void setup() override;
	void mouseDrag( MouseEvent event ) override;
    void touchesMoved( ci::app::TouchEvent event ) override;
    void keyDown(KeyEvent event) override;
	void update() override;
	void draw() override;
 
    void splat(vec2 pos, vec2 vel)
    {
        mFluid->splatVelocity( pos, vec4(vel.x, vel.y, 0.0f, 0.0f), 2.0f);
        mFluid->splatRgb( pos, vec4(0.0f, 0.0f, 0.0f, 1.0f) * 500.0f, 2.0f );
    }
 
    const ivec2                 mRes = { 512, 512 };
    std::shared_ptr<GPUFluid>   mFluid;
    vec2 mPrevPos = vec2(0.0f);
};

void GPUFluidApp::setup()
{
    mFluid = std::make_shared<GPUFluid>(mRes, getWindowAspectRatio());
}

void GPUFluidApp::keyDown(cinder::app::KeyEvent event)
{
    if (event.getChar() == 's')
    {
        auto surf = mFluid->getRgbTexture()->createSource();
    
        auto path = getAssetPath("") / "test.png";
        writeImage(path, surf);
    }
    else if (event.getCode() == KeyEvent::KEY_ESCAPE)
    {
        quit();
    }
}

void GPUFluidApp::mouseDrag( MouseEvent event )
{
    const vec2 pos = (vec2)event.getPos() / (vec2)getWindowSize();
    const vec2 vel = (pos - mPrevPos) * 1000000.0f;
    
    mFluid->splatVelocity(pos, vec4(vel.x, -vel.y, 0.0f, 0.0f), 2.0f);
    mFluid->splatRgb(pos, vec4(0.0f, 0.0f, 0.1f, 1.0f) * 500.0f, 2.0f);

    mPrevPos = pos;
}

void GPUFluidApp::touchesMoved( TouchEvent event )
{
    const std::vector<TouchEvent::Touch>& touches = event.getTouches();
    
    for( std::vector<TouchEvent::Touch>::const_iterator cit = touches.begin(); cit != touches.end(); ++cit )
    {
        vec2 prevPos = cit->getPrevPos() / (vec2)getWindowSize();
        vec2 pos = cit->getPos() / (vec2)getWindowSize();
        
        vec2 dv = pos - prevPos;
        vec2 vel = dv * 1000000.0f;
        
        splat(pos, vel);
    }
}


void GPUFluidApp::update()
{
    mFluid->step();
}

void GPUFluidApp::draw()
{
	gl::clear( Color( 0, 0, 0) );
 
    gl::ScopedColor colScp(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
    gl::drawSolidRect(getWindowBounds());
    gl::draw(mFluid->getRgbTexture(), getWindowBounds());
}

CINDER_APP( GPUFluidApp, RendererGl, [&](App::Settings* settings)
{
    settings->setHighDensityDisplayEnabled();
    settings->setWindowSize(700, 700);
    settings->setMultiTouchEnabled();
})
