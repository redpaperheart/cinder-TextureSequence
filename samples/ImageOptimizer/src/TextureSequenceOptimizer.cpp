//
//  TextureSequenceOptimizer.cpp
//
//  Created by Daniel Scheibel on 12/8/15.
//

#include "TextureSequenceOptimizer.h"

TextureSequenceOptimizer::TextureSequenceOptimizer(){
    ci::app::console() << ci::app::getElapsedSeconds() << " TextureSequenceOptimizer" << std::endl;
    
}

void TextureSequenceOptimizer::setup( const fs::path& path )
{
    m = Model::getInstance();
    
    loadImageDirectory( path );
    renderImagesToFbo();
//    play( event.getFile( s ) );
}

void TextureSequenceOptimizer::renderImagesToFbo()
{
    // save size of loaded images
    int width = mTextureRefs[0]->getWidth();
    int height = mTextureRefs[0]->getHeight();
    
    mOriOutline = Area(0,0,width, height);
    
    // create fbo
    mFboRef = gl::Fbo::create(width, height, true);
    
    {
        // bind fbo
        gl::ScopedFramebuffer fbScp( mFboRef );
        
        // set viewport and matrices
        gl::ScopedViewport scpVp( ivec2( 0 ), mFboRef->getSize() );
        gl::ScopedMatrices matricesFbo;
        gl::setMatricesWindow(width, height);
        
        // clear fbo and draw images
        gl::clear(ColorA(0, 0, 0, 0));
        gl::color(1, 1, 1);
        
        // draw images on fbo
        for (gl::TextureRef tex: mTextureRefs) {
            gl::draw(tex);
        }
        
    }
    //read overlayed texuture from fbo
    mResultTextureRef = mFboRef->getColorTexture();
    
    trim();
}

//void TextureSequenceOptimizer::resize(){}

ci::gl::TextureRef TextureSequenceOptimizer::load( const std::string &url, ci::gl::Texture::Format fmt )
{
    try{
        ci::gl::TextureRef t = ci::gl::Texture::create( ci::loadImage( url ), fmt );
        return t;
    }catch(...){}
    ci::app::console() << ci::app::getElapsedSeconds() << ": error loading texture '" << url << "'!" << std::endl;
    return NULL;
}

std::vector<ci::gl::TextureRef> TextureSequenceOptimizer::loadImageDirectory(ci::fs::path dir, ci::gl::Texture::Format fmt){
    
    bTrimmedMax = false;
    
    mTextureRefs.clear();
    mSurfaceRefs.clear();
    mFileNames.clear();
    
    std::vector<ci::gl::TextureRef> textureRefs;
    textureRefs.clear();
    for ( ci::fs::directory_iterator it( dir ); it != ci::fs::directory_iterator(); ++it ){
        if ( ci::fs::is_regular_file( *it ) ){
            // -- Perhaps there is  a better way to ignore hidden files
            ci::fs::path fileExtention = it->path().extension();
            std::string fileName = it->path().filename().string();
            //load acceptable images only
            if( fileExtention == ".png" || fileExtention == ".jpg" || fileExtention == ".jpeg" ){
                // load dropped images
                std::string path = dir.string() + "/" + fileName;
                SurfaceRef surf = Surface::create(loadImage(path));
                gl::TextureRef tex = gl::Texture::create(*surf);
                
                // save them in vector
                mTextureRefs.push_back(tex);
                mSurfaceRefs.push_back(surf);
                
                //save the names in vector
                mFileNames.push_back(fileName);
            }
        }
    }
    return textureRefs;
}

void TextureSequenceOptimizer::trim(){
    trimMax();
    trimMin();
    
}


//trim to the max amount for each image.
void TextureSequenceOptimizer::trimMax(){
    
    mTrimMaxAreas.clear();
    
    // number of lines to cut
    int trimTop = 0;
    int trimBottom = 0;
    int trimLeft = 0;
    int trimRight = 0;
    
    //get the pixels need to be trimmed for each surface with 4 loops
    for (SurfaceRef surf : mSurfaceRefs) {
        bool stop = false;
        //go thru pixels from top
        for (int y = 0; y < surf->getHeight(); y++) {
            for (int x =0; x < surf->getWidth(); x++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                //stop at the first non-transparent pixel
                if ( c.a > 0.0f ) {
                    trimTop = y;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //go thru from bottom
        stop = false;
        for (int y = surf->getHeight() -1; y >=  0; y--) {
            for (int x =0; x < surf->getWidth(); x++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimBottom = y+1;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //go thru from left
        stop = false;
        for (int x = 0; x < surf-> getWidth(); x++) {
            for (int y =0; y < surf->getHeight(); y++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimLeft = x;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //from right
        stop = false;
        for (int x = surf->getWidth()-1; x >= 0; x--) {
            for (int y =0; y < surf->getHeight(); y++) {
                ci::ColorA c = surf->getPixel( vec2(x, y) );
                if ( c.a > 0.0f ) {
                    trimRight = x+1;
                    stop = true;
                    break;
                }
            }
            if( stop ) break;
        }
        
        //pixel offsets need to be trimmed for each image
        //push to vector holds all the trim offsets
        mTrimMaxAreas.push_back( Area(trimLeft, trimTop, trimRight, trimBottom) );
        
    }
    bTrimmedMax = true;
}

//void TextureSequenceOptimizer::showMaxTrim(){
//    bMinTrim = false;
//    if (bTrimmed) {
//        bMaxTrim = true;
//    }else{
//        trim();
//        bMaxTrim = true;
//    }
//}

//trim minimum amount of pixels
void TextureSequenceOptimizer::trimMin(){
    
    if (!bTrimmedMax) {
        trimMax();
    }
    int tempX1 = mTrimMaxAreas[0].x1;
    int tempY1 = mTrimMaxAreas[0].y1;
    int tempX2 = 0;
    int tempY2 = 0;
    
    for (int i =1; i < mTrimMaxAreas.size(); i++) {
        if (mTrimMaxAreas[i].x1 < tempX1) {
            tempX1 = mTrimMaxAreas[i].x1;
        }
        
        if (mTrimMaxAreas[i].y1 < tempY1) {
            tempY1 = mTrimMaxAreas[i].y1;
        }
    }
    
    for (int i =0; i < mTrimMaxAreas.size(); i++) {
        if (mTrimMaxAreas[i].x2 > tempX2) {
            tempX2 = mTrimMaxAreas[i].x2;
        }
        
        if (mTrimMaxAreas[i].y2 > tempY2) {
            tempY2 = mTrimMaxAreas[i].y2;
        }
    }
    
    mTrimMinArea = Area(tempX1, tempY1, tempX2, tempY2);
//    cinder::app::console() << "mTrimArea: " << mTrimMinArea<<std::endl;
//    bMaxTrim = false;
//    bMinTrim = true;
    
}

//void TextureSequenceOptimizer::showBoth(){
//    bMaxTrim = true;
//    bMinTrim = true;
//}

void TextureSequenceOptimizer::saveMax( fs::path path ){
    if(path == fs::path()){
        path = ci::app::App::get()->getFolderPath();
        ci::app::console() << "SAVE MAX: " << path << std::endl;
    }
    if( ! path.empty() ){
        //go thru each surface
        for (int i = 0; i < mSurfaceRefs.size(); i++) {
            
            ci::fs::path tempPath = path;
            tempPath.append(toString(mFileNames[i]));
            
            //only clone the non-transparent area based on the offsets
            Surface tempSurf;
            if( mTrimMaxAreas[i].calcArea() == 0 ){
                ci::app::console() << " Image is completely transparent: " << tempPath << std::endl;
                tempSurf = mSurfaceRefs[i]->clone( Area(0,0,10,10) );
            }else{
                tempSurf = mSurfaceRefs[i]->clone(mTrimMaxAreas[i]);
            }
            
//            ci::app::console() << "saving: " << tempPath << " "<< mTrimMaxAreas[i] << std::endl;
            writeImage( tempPath, tempSurf );
            tempPath.clear();
        }
        saveJson(path);
    }
}

void TextureSequenceOptimizer::saveMin( fs::path path ){
    if(path == fs::path()){
        path = ci::app::App::get()->getFolderPath();
    }
    ci::app::console() << "SAVE MIN: " << path << std::endl;
    //go thru each surface
    for (int i = 0; i < mSurfaceRefs.size();i++) {
        //only clone the non-transparent area based on the offsets
        Surface tempSurf = mSurfaceRefs[i]->clone(mTrimMinArea);
        ci::fs::path tempPath = path;
        tempPath.append(toString(mFileNames[i]));
        //save them to desktop folder trimmed
        writeImage( tempPath, tempSurf);
        tempPath.clear();
    }
}

void TextureSequenceOptimizer::saveJson(const fs::path& path){
    //save the offsets for each image into a json file
    JsonTree doc = JsonTree::makeObject();
    JsonTree sequence = JsonTree::makeArray("sequence");
    //    fs::path jsonPath  = getHomeDirectory() / "Desktop" / "trimmed"/ "max" / "sequence.json";
    fs::path jsonPath  = path;
    jsonPath.append("sequence.json");
    
    for (int i = 0; i < mTrimMaxAreas.size(); i ++) {
        JsonTree curImage = JsonTree::makeObject();
        curImage.pushBack(JsonTree("x", mTrimMaxAreas[i].x1));
        curImage.pushBack(JsonTree("y", mTrimMaxAreas[i].y1));
        curImage.pushBack(JsonTree("fileName", mFileNames[i] ));
        sequence.pushBack(curImage);
    }
    doc.pushBack(sequence);
    doc.write( jsonPath, JsonTree::WriteOptions());
}

//void TextureSequenceOptimizer::showAnimation(){
//    bPlay = true;
//}

//void TextureSequenceOptimizer::play(const fs::path& path){
//    
//    mSequence = new rph::TextureSequence();
//    mSequence->setup( loadImageDirectory( path ) );
//    mSequence->setLoop(true);
//    if (bPlay) {
//        mSequence->play();
//    }
//};


void TextureSequenceOptimizer::update()
{
//    ci::app::console() << "Loaded amd Trimmed. Surfaces: " << mSurfaces.size() << ", Filenames: " << mFileNames.size() << std::endl;
    
//    if(mSequence){
//        mSequence->update();
//    }
}

void TextureSequenceOptimizer::draw()
{
    gl::color(1,1,1);
    gl::draw(mResultTextureRef);

    gl::color( ColorA(1, 0, 0, 0.5f) );
    gl::drawSolidRect(ci::Rectf(mOriOutline.x1,mOriOutline.y1, mOriOutline.x2, mTrimMinArea.y1));
    gl::drawSolidRect(ci::Rectf(mOriOutline.x1,mOriOutline.y1, mTrimMinArea.x1, mOriOutline.y2));
    gl::drawSolidRect(ci::Rectf(mOriOutline.x1,mTrimMinArea.y2, mOriOutline.x2, mOriOutline.y2));
    gl::drawSolidRect(ci::Rectf(mTrimMinArea.x2,mOriOutline.y1, mOriOutline.x2, mOriOutline.y2));

    gl::color(0,0,1);
    for (Area trimArea : mTrimMaxAreas) {
        gl::drawStrokedRect(ci::Rectf(trimArea));
    }
    
//    if(mSequence){
//        gl::color(ColorA(1,1,1,1));
//        gl::draw( mSequence->getCurrentTexture() );
//    }
    
    gl::color(1, 0, 0);
    gl::drawStrokedRect(ci::Rectf(mOriOutline));
    
}

