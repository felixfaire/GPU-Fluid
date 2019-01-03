//
//  GPUFluid.cpp
//  GPUFluid
//
//  Created by Felix Faire on 09/04/2018.
//

#include "GPUFluid.h"

GPUFluid::GPUFluid(ivec2 resolution, float targetAspectRatio)
        : mResolution(resolution),
          mTargetAspectRatio(targetAspectRatio)
{
    const float resolutionAspect = (float)resolution.x / (float)resolution.y;
    mCellSize = vec2(1.0f) / (vec2)resolution;
    mHalfDivCellSize = vec2(0.5f) / mCellSize;
    
    auto geom = geom::Rect(Rectf(0.0f, 0.0f, 1.0f, 1.0f));
    
    // JACOBI SOLVER
    {
        auto jacobiGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("jacobi.frag"));
        jacobiGlsl->uniform("u_XMat", 0);
        jacobiGlsl->uniform("u_BMat", 1);
        jacobiGlsl->uniform("u_CellSize", mCellSize * 2.0f);
        mJacobiBatch = gl::Batch::create(geom, jacobiGlsl);
    }
    
    // ADVECTION SOLVER
    {
        auto advectGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("advect.frag"));
        advectGlsl->uniform("u_Src", 0);
        advectGlsl->uniform("u_Vel", 1);
        advectGlsl->uniform("u_CellSize", mCellSize);
        advectGlsl->uniform("u_Resolution", (vec2)mResolution);
        mAdvectBatch = gl::Batch::create(geom, advectGlsl);
    }
    
    // DIVERGENCE SOLVER
    {
        auto divergenceGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("divergence.frag"));
        divergenceGlsl->uniform("u_Vel", 0);
        divergenceGlsl->uniform("u_CellSize", mCellSize);
        divergenceGlsl->uniform("u_HalfDivCellSize", mHalfDivCellSize);
        mDivergenceBatch = gl::Batch::create(geom, divergenceGlsl);
    }
    
    // SUBTRACT GRADIENT SOLVER
    {
        auto subtractGradientGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("subtract_gradient.frag"));
        subtractGradientGlsl->uniform("u_Vel", 0);
        subtractGradientGlsl->uniform("u_Pressure", 1);
        subtractGradientGlsl->uniform("u_CellSize", mCellSize);
        subtractGradientGlsl->uniform("u_HalfDivCellSize", mHalfDivCellSize);
        mSubtractGradientBatch = gl::Batch::create(geom, subtractGradientGlsl);
    }
    
    // CURL SOLVER
    {
        auto curlGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("calculate_curl.frag"));
        curlGlsl->uniform("u_Vel", 0);
        curlGlsl->uniform("u_CellSize", mCellSize);
        mCurlBatch = gl::Batch::create(geom, curlGlsl);
    }
    
    // VORTICITY CONFINEMENT SOLVER
    {
        auto vorticityGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("vorticity_confinement.frag"));
        vorticityGlsl->uniform("u_Vel", 0);
        vorticityGlsl->uniform("u_Crl", 1);
        vorticityGlsl->uniform("u_CellSize", mCellSize);
        mVorticityBatch = gl::Batch::create(geom, vorticityGlsl);
    }
    
    // SPLAT
    {
        auto splatGlsl = gl::GlslProg::create(loadAsset("thru.vert"), loadAsset("splat.frag"));
        splatGlsl->uniform("u_CellSize", mCellSize);
        mSplatBatch = gl::Batch::create(geom, splatGlsl);
    }
    
    auto texFmt = gl::Texture::Format();
    texFmt.internalFormat(GL_RGBA32F);
    texFmt.mipmap(false);
    const auto dataFmt = gl::Fbo::Format().colorTexture(texFmt);
    
    auto texFmt1D = gl::Texture::Format();
    texFmt1D.internalFormat(GL_R32F);
    texFmt1D.mipmap(false);
    const auto dataFmt1D = gl::Fbo::Format().colorTexture(texFmt1D);
    
    // FBOS
    for (int i = 0; i < 2; ++i)
    {
        mVel[i] = gl::Fbo::create(mResolution.x, mResolution.y, dataFmt);
        mRgb[i] = gl::Fbo::create(mResolution.x, mResolution.y, dataFmt);
        mPressure[i] = gl::Fbo::create(mResolution.x, mResolution.y, dataFmt1D);
    }
    
    mDivergence = gl::Fbo::create(mResolution.x, mResolution.y, dataFmt1D);
    mCurl = gl::Fbo::create(mResolution.x, mResolution.y, dataFmt);
}



void GPUFluid::splatFbo(gl::FboRef& target, vec2 pos, vec4 splat, float radius)
{
    gl::ScopedFramebuffer targetScp(target);
    gl::ScopedViewport    viewScp(target->getSize());
    gl::ScopedMatrices    matScp;
    gl::setMatricesWindow(ivec2(1));
    gl::ScopedBlend blendScp(GL_ONE, GL_ONE);
    
    mSplatBatch->getGlslProg()->uniform("u_Pos", pos);
    mSplatBatch->getGlslProg()->uniform("u_Splat", splat);
    mSplatBatch->getGlslProg()->uniform("u_Radius", radius);
    mSplatBatch->getGlslProg()->uniform("u_TargetAspect", mTargetAspectRatio);
    mSplatBatch->draw();
}

void GPUFluid::jacobi2DSingle(float             alpha,
                              float             beta,
                              const gl::FboRef& xMat,
                              const gl::FboRef& bMat,
                              gl::FboRef&       outMat)
{
    const float invBeta = 1.0f / beta;
    
    gl::ScopedFramebuffer fboScp(outMat);
    gl::ScopedTextureBind xmat(xMat->getColorTexture(), 0);
    gl::ScopedTextureBind bmat(bMat->getColorTexture(), 1);

    mJacobiBatch->getGlslProg()->uniform("u_Alpha", alpha);
    mJacobiBatch->getGlslProg()->uniform("u_InvBeta", invBeta);
    mJacobiBatch->draw();
}

void GPUFluid::jacobi2D(float             alpha,
                        float             beta,
                        const gl::FboRef& xMat,
                        const gl::FboRef& bMat,
                        gl::FboRef&       outMat,
                        int               numIters)
{
    for( int solveIter = 0; solveIter < numIters; ++solveIter )
        jacobi2DSingle(alpha, beta, xMat, bMat, outMat);
}

void GPUFluid::diffuse2D(float                viscosity,
                         float                dt,
                         const gl::FboRef&    src,
                         gl::FboRef&          dst,
                         int                  numIters)
{
    const float alpha = mCellSize.x * mCellSize.y / (viscosity * dt);
    const float beta = 4.0f + alpha;
    jacobi2D(alpha, beta, src, src, dst, numIters);
}

void GPUFluid::advect2D(float                        dissipation,
                        float                        dt,
                        const gl::FboRef&            src,
                        const gl::FboRef&            vel,
                        gl::FboRef&                  dst)
{
    gl::ScopedFramebuffer fboScp(dst);
    gl::ScopedTextureBind srcTex(src->getColorTexture(), 0);
    gl::ScopedTextureBind velTex(vel->getColorTexture(), 1);

    mAdvectBatch->getGlslProg()->uniform("u_Dt", dt);
    mAdvectBatch->getGlslProg()->uniform("u_Dissipation", dissipation);
    mAdvectBatch->draw();
}

void GPUFluid::computeDivergence2D(const gl::FboRef& vel,
                                   gl::FboRef& outDivergence)
{
    gl::ScopedFramebuffer fboScp(outDivergence);
    gl::ScopedTextureBind srcTex(vel->getColorTexture(), 0);

    mDivergenceBatch->draw();
}

void GPUFluid::solvePressure2D(int               numIters,
                               const gl::FboRef& divergence,
                               gl::FboRef&       inPressure,
                               gl::FboRef&       outPressure)
{
    float alpha = -mCellSize.x * mCellSize.y;
    float beta = 4.0f;
    
    // Clear pressures
    {
        gl::ScopedFramebuffer inScp(inPressure);
        gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
    
        gl::ScopedFramebuffer outScp(outPressure);
        gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
    }

    for (int iter = 0; iter < numIters; ++iter)
    {
        jacobi2DSingle(alpha, beta, inPressure, divergence, outPressure);
        std::swap(inPressure, outPressure);
    }
}

void GPUFluid::subtractGradient2D(const gl::FboRef& pressure,
                                  const gl::FboRef& inVel,
                                  gl::FboRef&       outVel)
{
    gl::ScopedFramebuffer fboScp(outVel);
    gl::ScopedTextureBind velTex(inVel->getColorTexture(), 0);
    gl::ScopedTextureBind pressureTex(pressure->getColorTexture(), 1);

    mSubtractGradientBatch->draw();
}

void GPUFluid::calculateCurlField2D(const gl::FboRef& inVel,
                                    gl::FboRef&       outCurl)
{
    gl::ScopedFramebuffer fboScp(outCurl);
    gl::ScopedTextureBind velTex(inVel->getColorTexture(), 0);

    mCurlBatch->draw();
}

void GPUFluid::vorticityConfinement2D(const gl::FboRef&    inVel,
                                      const gl::FboRef&    inCurl,
                                      gl::FboRef&          outVel)
{
    gl::ScopedFramebuffer fboScp(outVel);
    gl::ScopedTextureBind velTex(inVel->getColorTexture(), 0);
    gl::ScopedTextureBind curlTex(inCurl->getColorTexture(), 1);

    mVorticityBatch->getGlslProg()->uniform("u_VorticityScale", mVorticityScale);
    mVorticityBatch->draw();
}

void GPUFluid::step()
{
    gl::ScopedBlend blendScp(false);
    gl::ScopedDepth depthScp(false);
    
    const float time = (float)getElapsedSeconds();
    const float dt = (time - mLastTime) * mTimeMultiplier;
    
    // Set simulation space
    gl::ScopedViewport    viewScp(mResolution);
    gl::ScopedMatrices    matScp;
    gl::setMatricesWindow(ivec2(1));
    
    // Velocity
    diffuse2D(mVelViscosity, dt, mVel[0], mVel[1], mNumDiffusionIters);
    std::swap(mVel[0], mVel[1]);
    advect2D(mVelDissipation, dt, mVel[0], mVel[0], mVel[1]);

    // RGB
    diffuse2D(mRgbViscosity, dt, mRgb[0], mRgb[1], mNumDiffusionIters);
    std::swap(mRgb[0], mRgb[1]);
    advect2D(mRgbDissipation, dt, mRgb[0], mVel[0], mRgb[1]);

    // Calculate divergence
    computeDivergence2D(mVel[1], mDivergence);
    
    // Solve pressure
    solvePressure2D(mNumPressureIters, mDivergence, mPressure[0], mPressure[1]);

    // Subtract gradient
    std::swap(mVel[0], mVel[1]);
    subtractGradient2D(mPressure[1], mVel[0], mVel[1]);

    // Vorticity confinement
    if(mEnableVorticityConfinement)
    {
        // Calculate curl field
        calculateCurlField2D(mVel[1], mCurl);

        // Vorticity confinement
        std::swap(mVel[0], mVel[1]);
        vorticityConfinement2D(mVel[0], mCurl, mVel[1]);
    }

    glFlush();

    std::swap(mVel[0], mVel[1]);
    std::swap(mRgb[0], mRgb[1]);
    std::swap(mPressure[0], mPressure[1]);
    mLastTime = time;
}
