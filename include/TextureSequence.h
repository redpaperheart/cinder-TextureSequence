#pragma once

#include <string>
#include <vector>

#include "cinder/app/App.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/Filesystem.h"

using namespace ci;
using namespace ci::gl;
using namespace ci::app;
using namespace std;

class TextureSequence{
public:
    TextureSequence();
    ~TextureSequence();
    
    void stop();
    void play();
    void pause();
    void update();
    void createFromDir(const string &path, const float &fps = 0.0f);
    void createFromPathList(const vector<string> &paths, const float &fps = 0.0f);
    void createFromTextureList(const vector<Texture *> &textureList, const float &fps = 0.0f);
    
    int getTotalFrames()const{ return totalFrames; } 
    
    int getPlayheadFrameInc() const { return playheadFrameInc; }
    void setPlayheadFrameInc( int frames ) { playheadFrameInc = frames; }
    
    int getPlayheadPosition() const { return playheadPosition; }
    void setPlayheadPosition( int newPosition );
    
    void setLooping( bool doLoop ) { looping = doLoop; }
    
    Texture* const getCurrentTexture() { return textures[ playheadPosition ]; }
    
    bool isPlaying() { return playing; }
    bool isPaused() { return paused; }
    
    //isDone returns true if sequence played thru and looping = false;
    bool isComplete()const{return complete;};
    
protected:
    
    
    
    int playheadPosition;
    int playheadFrameInc;
    vector<Texture *> textures;
    
    int totalFrames;
    bool looping;
    bool paused;
    bool playing;
    bool complete;
    
    float mStartTime;
    float mFps;
    
};