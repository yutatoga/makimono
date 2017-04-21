#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // gui
    // we add this listener before setting up so the initial circle resolution is correct
    movingSpeed.addListener(this, &ofApp::movingSpeedChanged);
    borderPosition.addListener(this, &ofApp::borderPositionChanged);
    
    showGui = true;
    panel.setup("panel", SETTING_FILE_NAME, GUI_POSITION_X, GUI_POSITION_Y);
    panel.add(fps.set("FPS", ""));
    panel.add(enableDrawAllPlayers.set("draw all players", false));
    panel.add(enableDrawBorder.set("draw border", false));
    panel.add(enableFullPlay.set("full play", false));
    panel.add(movingSpeed.set("moving speed", ofVec2f(1, 0), ofVec2f(SPEED_MINIMUM_X, SPEED_MINIMUM_Y), ofVec2f(SPEED_MAXIMUM_X, SPEED_MAXIMUM_Y)));
    panel.add(windowSize.set("window size", 0.5, 0, 1));
    panel.add(borderPosition.set("border position", 0.5, 0, 1));
    panel.add(shortCutInfo.setup("hide/show GUI", "type h"));
    
    //some path, may be absolute or relative to bin/data
    string path = "movies";
    ofDirectory dir(path);
    //only show video files
    dir.allowExt("mov");
    dir.allowExt("mp4");
    //populate the directory object
    dir.listDir();
    if (dir.size() == 0) ofLogError("can't find any video files");
    
    scrollPlayer.rightViewId = 0;
    
    //go through and print out all the paths
    for(int i = 0; i < (int)dir.size(); i++){
        // set scrollPlayer
        shared_ptr<MovingVideoPlayer> player = std::make_shared<MovingVideoPlayer>();
        player->load(dir.getPath(i));
        player->setLoopState(OF_LOOP_NONE);
        player->stop();
        scrollPlayer.players.push_back(player);
        
        // debug
        ofLogNotice(dir.getPath(i));
    }
    scrollPlayer.rightViewPosition.set(0, 0);
    scrollPlayer.movingSpeed.set(movingSpeed);
    
    // set window
    if (scrollPlayer.players[0]->isLoaded()) {
        ofSetWindowShape(scrollPlayer.players[0]->getWidth(), scrollPlayer.players[0]->getHeight());
        ofSetWindowPosition(0, 0);
    }
    
    ofSetFrameRate(FRAME_RATE);
}

void ofApp::movingSpeedChanged(ofVec2f &movingSpeed){
    scrollPlayer.movingSpeed = movingSpeed;
}

void ofApp::borderPositionChanged(float &borderPosition){
    ofDrawCircle(100, 100, 100);
}

//--------------------------------------------------------------
void ofApp::update(){
    // update position
    scrollPlayer.rightViewPosition += scrollPlayer.movingSpeed * (ofGetTargetFrameRate() * ofGetLastFrameTime());

    // when the rightViewPosition has reached or stepped over the end position
    if(scrollPlayer.rightViewPosition.x >= scrollPlayer.players[0]->getWidth()){
        // stop right(old) player
        scrollPlayer.players[scrollPlayer.rightViewId]->stop();
        // update rightViewId
        scrollPlayer.rightViewId = (scrollPlayer.rightViewId+1)%(int)scrollPlayer.players.size();
        // update rightViewPosition
        scrollPlayer.rightViewPosition -= ofVec2f(scrollPlayer.players[0]->getWidth(), 0);
    }
    
    // update 3 of players which are right view, left view, standby view.
    for(int i = scrollPlayer.rightViewId; i < scrollPlayer.rightViewId+3; i++){
        int playerIndex = i%(int)scrollPlayer.players.size();
        // when the view has reached or stepped over the borderPosition
        if (!scrollPlayer.players[playerIndex]->isPlaying() && !scrollPlayer.players[playerIndex]->getIsMovieDone() && scrollPlayer.rightViewPosition.x - scrollPlayer.players[0]->getWidth() * (i-scrollPlayer.rightViewId-1) > borderPosition*ofGetWidth()){
            // set playback speed
            if (enableFullPlay) {
                float playbackSpeed = scrollPlayer.players[playerIndex]->getDuration() * scrollPlayer.movingSpeed.x * ofGetTargetFrameRate() / scrollPlayer.players[playerIndex]->getWidth();
                scrollPlayer.players[playerIndex]->setSpeed(playbackSpeed);
            }
            // unmute the sound of the player
            scrollPlayer.players[playerIndex]->setVolume(1.0);
            // play the player
            scrollPlayer.players[playerIndex]->play();
        }
        
        // when the view has passed the borderPosition
        if (scrollPlayer.players[playerIndex]->isPlaying() && !scrollPlayer.players[playerIndex]->isMuted && scrollPlayer.rightViewPosition.x - scrollPlayer.players[0]->getWidth() * (i-scrollPlayer.rightViewId) > borderPosition*ofGetWidth()){
            // mute the sound of the player
            scrollPlayer.players[playerIndex]->setVolume(0.0);
        }
        
        // update player
        scrollPlayer.players[playerIndex]->update();
    }
    
    // gui
    fps = ofToString(ofGetFrameRate(), 0);
}

//--------------------------------------------------------------
void ofApp::draw(){
    // draw 3 of players which are right view, left view, stadnby view.
    for(int i = scrollPlayer.rightViewId; i < scrollPlayer.rightViewId+3; i++){
        int playerIndex = i%(int)scrollPlayer.players.size();
        ofVec2f playerPosition(scrollPlayer.rightViewPosition-(i-scrollPlayer.rightViewId)*ofVec2f(scrollPlayer.players[0]->getWidth(),0));
        scrollPlayer.players[playerIndex]->draw(playerPosition.x, playerPosition.y);
    }
    
    // debug
    if(enableDrawAllPlayers)ofApp::drawAllPlayers();
    if(enableDrawBorder)ofApp::drawBorder(ofColor::red);
    
    // gui
    if(showGui)panel.draw();
}

ofVec2f ofApp::getPlayerPosition(int playerIndex) {
    ofVec2f origin = panel.getShape().getBottomLeft() + ofPoint(0, 10);
    return (ofVec2f(origin.x+playerIndex%5*(DEBUG_PLAYER_WIDTH+20), origin.y+playerIndex/5*(DEBUG_PLAYER_HEIGHT+20)));
}

void ofApp::drawAllPlayers(){
    // draw background
    // rightView
    ofSetColor(255);
    ofVec2f rightViewPosition = getPlayerPosition(scrollPlayer.rightViewId);
    ofDrawRectangle(rightViewPosition.x-1, rightViewPosition.y-1, DEBUG_PLAYER_WIDTH+2, DEBUG_PLAYER_HEIGHT+2);
    
    // second right view
    int secondRightViewId = (scrollPlayer.rightViewId+1)%(int)scrollPlayer.players.size();
    ofVec2f secondRightViewPosition = getPlayerPosition(secondRightViewId);
    ofSetColor(127);
    ofDrawRectangle(secondRightViewPosition.x-1, secondRightViewPosition.y-1, DEBUG_PLAYER_WIDTH+2, DEBUG_PLAYER_HEIGHT+2);
    
    // third right view
    int thirdRightViewId = (scrollPlayer.rightViewId+2)%(int)scrollPlayer.players.size();
    ofVec2f thirdRightViewPosition = getPlayerPosition(thirdRightViewId);
    ofSetColor(64);
    ofDrawRectangle(thirdRightViewPosition.x-1, thirdRightViewPosition.y-1, DEBUG_PLAYER_WIDTH+2, DEBUG_PLAYER_HEIGHT+2);
    
    // reset the color
    ofSetColor(255);
    
    // draw players
    for (int i = 0; i < (int)scrollPlayer.players.size(); i++){
        if (scrollPlayer.players[i]->isLoaded()) {
            ofVec2f playerPosition = getPlayerPosition(i);
            scrollPlayer.players[i]->draw(playerPosition.x, playerPosition.y, DEBUG_PLAYER_WIDTH, DEBUG_PLAYER_HEIGHT);
        }
    }
}

void ofApp::drawBorder(ofColor lineColor){
    ofSetColor(lineColor);
    ofDrawLine(ofGetWidth()*borderPosition, 0, ofGetWidth()*borderPosition, ofGetHeight());
    ofSetColor(255);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 'h': // hide or show gui
            showGui = !showGui;
            showGui ? ofShowCursor() : ofHideCursor();
            break;
        case 's': // save the gui setting
            panel.saveToFile(SETTING_FILE_NAME);
            break;
        case 'l': // load the gui setting
            panel.loadFromFile(SETTING_FILE_NAME);
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
