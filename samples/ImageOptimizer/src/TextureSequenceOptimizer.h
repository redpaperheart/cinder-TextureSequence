//
//  TextureSequenceOptimizer.h
//
//  Created by Daniel Scheibel on 12/8/15.
//

#pragma once
#include "cinder/app/App.h"

#include "Model.h"
#include "TextureSequence.h"

using namespace ci;

class TextureSequenceOptimizer
{
  public:
    TextureSequenceOptimizer();
    void setup( const fs::path& path );
    void update();
    void draw();

    
    void saveMax( fs::path path = fs::path() );
    void saveMin( fs::path path = fs::path() );
    
//    void showAnimation();
//    void play(const fs::path& path);
    
  protected:
    
    void loadImages(const fs::path& path );
    void renderImagesToFbo();
    void trim();
    void trimMax();
    void trimMin();
    
    void saveJson(const fs::path& path);
    
    Model           *m;
    
    bool bTrimmedMax = false;
    
    rph::TextureSequence *mSequence = NULL;//texture sequencer
    
    Area                mTrimMinArea = Area(0,0,0,0);
    Area                mOriOutline = Area(0,0,0,0);
    gl::TextureRef      mResultTextureRef = NULL;
    gl::FboRef          mFboRef = NULL;
    
    std::vector<gl::TextureRef>     mTextureRefs; //texture vector holds all the dropped images for overlaped display
    std::vector<SurfaceRef>         mSurfaceRefs; //surfaces to hold all the images for trimming
    std::vector<std::string>        mFileNames; //vector to store origin file names
    std::vector<Area>               mTrimMaxAreas; //the amount of pixels to trim for each image
    
    
    ci::gl::TextureRef load( const std::string &url, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
    std::vector<ci::gl::TextureRef> loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt = ci::gl::Texture::Format());
};