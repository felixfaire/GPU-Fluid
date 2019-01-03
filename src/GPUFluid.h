//
//  GPUFluid.h
//  GPUFluid
//
//  Created by Felix Faire on 05/04/2018.
//

#ifndef GPUFluid_h
#define GPUFluid_h

#include "cinder/gl/gl.h"
#include "cinder/app/App.h"

using namespace ci;
using namespace ci::app;

/*  GPU Fluid sim based on Hai Ngyuen's CPU implementation.

    Note: Simulation space should be same aspect ratio as 
    the target texture aspect ratio (but preferably square).
*/
class GPUFluid
{
public:
    GPUFluid(ivec2 internalResolution, float targetAspectRatio = 1.0f);
    
    void splatRgb(vec2 pos, ColorA splat, float radius)      { splatFbo(mRgb[0], pos, (vec4)splat, radius); }
    void splatVelocity(vec2 pos, vec2 splat, float radius)   { splatFbo(mVel[0], pos, vec4(splat.x, -splat.y, 0.0f, 0.0f), radius); }
    
    void step();
    
    ivec2                 getResolution() const { return mResolution; }
    float                 getAspectRatio() const { return mResolution.x / mResolution.y; }
    
    const gl::TextureRef  getRgbTexture() const      { return mRgb[0]->getColorTexture(); }
    const gl::TextureRef  getVelocityTexture() const { return mVel[0]->getColorTexture(); }
    const gl::TextureRef  getCurlTexture() const     { return mCurl->getColorTexture(); }
    const gl::TextureRef  getPressureTexture() const { return mPressure[0]->getColorTexture(); }
    
    void enableVorticityConfinement(bool enable = true) { mEnableVorticityConfinement = enable; }
    
private:

    ivec2           mResolution;
    float           mTargetAspectRatio;
    vec2            mCellSize;
    vec2            mHalfDivCellSize;
    int             mNumPressureIters = 10;
    int             mNumDiffusionIters = 2;
    
    gl::BatchRef    mJacobiBatch;
    gl::BatchRef    mAdvectBatch;
    gl::BatchRef    mDivergenceBatch;
    gl::BatchRef    mSubtractGradientBatch;
    gl::BatchRef    mSplatBatch;
    gl::BatchRef    mCurlBatch;
    gl::BatchRef    mVorticityBatch;
    
    gl::FboRef      mVel [2];
    gl::FboRef      mRgb [2];
    gl::FboRef      mPressure [2];
    gl::FboRef      mDivergence;
    gl::FboRef      mCurl;
    
    float           mLastTime = 0.0f;
    float           mTimeMultiplier = 5.0f;
    
    bool            mEnableVorticityConfinement = true;
    float           mVorticityScale = 0.005f;
    
    float           mVelViscosity = 0.00000002f;
    float           mVelDissipation = 0.997f;
    
    float           mRgbViscosity = 0.00000002f;
    float           mRgbDissipation = 0.994f;
    
    void splatFbo(gl::FboRef& target, vec2 pos, vec4 splat, float radius);

    void jacobi2DSingle(float alpha, float beta, const gl::FboRef& xMat, const gl::FboRef& bMat, gl::FboRef& outMat);
    void jacobi2D(float alpha, float beta, const gl::FboRef& xMat, const gl::FboRef& bMat, gl::FboRef& outMat, int numIters);
    void diffuse2D(float viscosity, float dt, const gl::FboRef& src, gl::FboRef& dst, int numIters);
    void advect2D(float dissipation, float dt, const gl::FboRef& src, const gl::FboRef& vel, gl::FboRef& dst);
    void computeDivergence2D(const gl::FboRef& vel, gl::FboRef& outDivergence);
    void solvePressure2D(int numIters, const gl::FboRef& divergence, gl::FboRef& inPressure, gl::FboRef& outPressure);
    void subtractGradient2D(const gl::FboRef& pressure, const gl::FboRef& inVel, gl::FboRef& outVel);
    void calculateCurlField2D(const gl::FboRef& inVel, gl::FboRef& outCurl);
    void vorticityConfinement2D(const gl::FboRef& inVel, const gl::FboRef& inCurl, gl::FboRef& outVel);
    
};

#endif /* GPUFluid_h */
