#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // gui
    // we add this listener before setting up gui
    movingSpeed.addListener(this, &ofApp::movingSpeedChanged);
    borderPosition.addListener(this, &ofApp::borderPositionChanged);
    recordingSwitch.addListener(this, &ofApp::recordingSwitchChanged);
    
    showGui = true;
    panel.setup("panel", SETTING_FILE_NAME, GUI_POSITION_X, GUI_POSITION_Y);
    panel.add(fps.set("FPS", ""));
    panel.add(enableDrawAllPlayers.set("draw all players", false));
    panel.add(enableDrawBorder.set("draw border", false));
    panel.add(enableDrawVideoGrabber.set("draw video grabber", false));
    panel.add(enableDrawVideoRecorderState.set("draw video recorder state", false));
    panel.add(enableFullPlay.set("full play", false));
    panel.add(movingSpeed.set("moving speed", ofVec2f(1, 0), ofVec2f(SPEED_MINIMUM_X, SPEED_MINIMUM_Y), ofVec2f(SPEED_MAXIMUM_X, SPEED_MAXIMUM_Y)));
    panel.add(windowSize.set("window size", 0.5, 0, 1));
    panel.add(borderPosition.set("border position", 0.5, 0, 1));
    panel.add(recordingSwitch.set("recording switch", false));
    panel.add(shortCutInfo.setup("hide/show GUI", "type h"));
    
    //some path, may be absolute or relative to bin/data
    ofDirectory dir(VIDEO_FOLDER_NAME);
    //only show video files
    dir.allowExt("mov");
    dir.allowExt("mp4");
    //populate the directory object
    dir.listDir();
    if (dir.size() == 0) ofLogError("can't find any video(.mov/.mp4) files");
    
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
        ofLogNotice(dir.getPath(i)) << endl;
    }
    scrollPlayer.rightViewPosition.set(0, 0);
    scrollPlayer.movingSpeed.set(movingSpeed);
    
    // set window
    if (scrollPlayer.players[0]->isLoaded()) {
        ofSetWindowShape(scrollPlayer.players[0]->getWidth(), scrollPlayer.players[0]->getHeight());
        ofSetWindowPosition(0, 0);
    } else {
        ofLogWarning("scrollPlayer.players[0] is not loaded");
    }
    
    ofSetFrameRate(FRAME_RATE);
    
    // videoGrabber
    videoGrabber.setDeviceID(VIDEO_DEVICE_ID);
    videoGrabber.setDesiredFrameRate(VIDEO_GRABBER_FRAME_RATE);
    videoGrabber.setup(ofGetWidth(), ofGetHeight());
    
    // video recorder
    ofApp::printVideoDeviceList(videoGrabber);
    videoRecorder.setVideoCodec(VIDEO_CODEC);
    videoRecorder.setVideoBitrate(VIDEO_BITRATE);
    videoRecorder.setAudioCodec(AUDIO_CODEC);
    videoRecorder.setAudioBitrate(AUDIO_BITRATE);
    ofAddListener(videoRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    
    // sound stream
    ofApp::printSoundDeviceList(soundStream);
    soundStream.setDeviceID(SOUND_DEVICE_ID);
    soundDevice = soundStream.getDeviceList()[SOUND_DEVICE_ID];
    soundStream.setup(this, soundDevice.outputChannels, soundDevice.inputChannels, soundDevice.sampleRates[SOUND_SAMPLE_RATES_INDEX], SOUND_BUFFER_SIZE, SOUND_NUMBER_OF_BUFFER);
    
    loading = false;
    
    cout << "----- finished setup -----" << endl << endl;
}

void ofApp::movingSpeedChanged(ofVec2f &movingSpeed){
    scrollPlayer.movingSpeed = movingSpeed;
}

void ofApp::borderPositionChanged(float &borderPosition){
    ofDrawCircle(100, 100, 100);
}

void ofApp::recordingSwitchChanged(bool &recordingSwitch){
    cout << "recordingSwitchChanged" << endl;
    if(recordingSwitch){
        if (videoRecorder.isInitialized()) {
            // restart recording
            videoRecorder.setPaused(false);
        } else {
            videoRecorder.setup(VIDEO_FOLDER_NAME + std::string("/") + VIDEO_FILE_NAME_HEADER+ofGetTimestampString()+VIDEO_FILE_NAME_EXTENSION, videoGrabber.getWidth(), videoGrabber.getHeight(), 30, soundDevice.sampleRates[SOUND_SAMPLE_RATES_INDEX], soundDevice.inputChannels);
            // start recording
            videoRecorder.start();
        }
    }else{
        if (videoRecorder.isInitialized()){
            if (ofGetKeyPressed(OF_KEY_SHIFT)) {
                // pause recording
                videoRecorder.setPaused(true);
            } else {
                // stop recording
                videoRecorder.close();
            }
        } else {
            ofLogWarning("do nothing");
        }
    }
}

void ofApp::audioIn(float *input, int bufferSize, int nChannels){
    if(recordingSwitch) videoRecorder.addAudioSamples(input, bufferSize, nChannels);
}

void ofApp::recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args){
    cout << "The recoded video file is now complete: " + args.fileName << endl;
    loading = true;
    loadingFileName = args.fileName;
}

void ofApp::exit(){
    movingSpeed.removeListener(this,  &ofApp::movingSpeedChanged);
    borderPosition.removeListener(this, &ofApp::borderPositionChanged);
    recordingSwitch.removeListener(this, &ofApp::recordingSwitchChanged);
    ofRemoveListener(videoRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);
    videoRecorder.close();
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
        int previousPlayerIndex = playerIndex-1;
        if (previousPlayerIndex < 0) previousPlayerIndex = scrollPlayer.players.size()-1; // = index of the last element
        // when the view has reached or stepped over the borderPosition
        if (!scrollPlayer.players[playerIndex]->isPlaying() && !scrollPlayer.players[playerIndex]->getIsMovieDone() && scrollPlayer.rightViewPosition.x - scrollPlayer.players[0]->getWidth() * (i-scrollPlayer.rightViewId-1) > borderPosition*ofGetWidth()){
            // set playback speed
            if (enableFullPlay) {
                float playbackSpeed = scrollPlayer.players[playerIndex]->getDuration() * scrollPlayer.movingSpeed.x * ofGetTargetFrameRate() / scrollPlayer.players[playerIndex]->getWidth();
                scrollPlayer.players[playerIndex]->setSpeed(playbackSpeed);
            }
            // mute the sound of the previous player
            scrollPlayer.players[previousPlayerIndex]->setVolume(0.0);
            // unmute the sound of the player
            scrollPlayer.players[playerIndex]->setVolume(1.0);
            // play the player
            scrollPlayer.players[playerIndex]->play();
        }
        
        // update player
        scrollPlayer.players[playerIndex]->update();
    }
    
    // videoGrabber
    videoGrabber.update();
    if (videoGrabber.isFrameNew() && recordingSwitch){
        bool success = videoRecorder.addFrame(videoGrabber.getPixels());
        if (!success) ofLogWarning("This frame was not added");
    }
    if (videoRecorder.hasVideoError()) ofLogWarning("The video recorder failed to write some frames");
    if (videoRecorder.hasAudioError()) ofLogWarning("The video recorder failed to write some audio samples");
    
    // gui
    fps = ofToString(ofGetFrameRate(), 0);
    
    // load the new video
    if (loading){
        shared_ptr<MovingVideoPlayer> player = std::make_shared<MovingVideoPlayer>();
        player->load(loadingFileName);
        if(player->isLoaded()){
            player->setLoopState(OF_LOOP_NONE);
            player->stop();
            scrollPlayer.players.push_back(player);
            loading = false;
            cout << "loaded the new video" << endl;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    // draw 3 of players which are right view, left view, standby view.
    for(int i = scrollPlayer.rightViewId; i < scrollPlayer.rightViewId+3; i++){
        int playerIndex = i%(int)scrollPlayer.players.size();
        ofVec2f playerPosition(scrollPlayer.rightViewPosition-(i-scrollPlayer.rightViewId)*ofVec2f(scrollPlayer.players[0]->getWidth(),0));
        scrollPlayer.players[playerIndex]->draw(playerPosition.x, playerPosition.y);
    }
    
    // debug
    if(enableDrawVideoGrabber) videoGrabber.draw(videoGrabber.getWidth()/2.0, 0, videoGrabber.getWidth()/2.0, videoGrabber.getHeight()/2.0);
    if(enableDrawVideoRecorderState) ofApp::drawVideoRecorderState();
    if(enableDrawAllPlayers) ofApp::drawAllPlayers();
    if(enableDrawBorder) ofApp::drawBorder(ofColor::red);
    
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

void ofApp::printVideoDeviceList(ofVideoGrabber videoGrabber){
    vector<ofVideoDevice> devices = videoGrabber.listDevices();
    cout << "----- VIDEO DEIVCES LIST -----" << endl;
    for(int i = 0; i < devices.size(); i++){
        cout << "deivce ID:\t\t" << devices[i].id << endl;
        cout << "devide name:\t\t"<< devices[i].deviceName << endl;
        cout << "hardware name:\t" << devices[i].hardwareName << endl;
        cout << "avairable:\t\t" << devices[i].bAvailable << endl;
        cout << "formats:" << endl;
        for (int j = 0; j < (int)devices[i].formats.size(); j++){
            cout << "\tpixel format:" << devices[i].formats[j].pixelFormat << endl;
            cout << "\twidth:\t" << devices[i].formats[j].width << endl;
            cout << "\theight:\t" << devices[i].formats[j].height << endl;
            for (int k = 0; k < (int)devices[i].formats[j].framerates.size(); k ++){
                cout << "\t\tframe rates:\t" << devices[i].formats[j].framerates[k] << endl;
            }
        }
        cout << endl;
    }
}

void ofApp::printSoundDeviceList(ofSoundStream soundStream){
    vector<ofSoundDevice> soundDevices = soundStream.getDeviceList();
    cout << "----- SOUND DEVICE LIST -----" << endl;
    for (int i = 0; i < soundDevices.size(); i++){
        cout << "device ID:\t\t" << soundDevices[i].deviceID << endl;
        cout << "device name:\t\t" << soundDevices[i].name << endl;
        cout << "input channels:\t" << soundDevices[i].inputChannels << endl;
        cout << "output channels:\t" << soundDevices[i].outputChannels << endl;
        cout << "default input:\t" << soundDevices[i].isDefaultInput << endl;
        cout << "default output:\t" << soundDevices[i].isDefaultOutput << endl;
        cout << "sample rates:\t\t";
        for (int j = 0; j < (int)soundDevices[i].sampleRates.size(); j++) {
            cout << soundDevices[i].sampleRates[j] << " ";
        }
        cout << "\n\n";
    }
}

void ofApp::drawVideoRecorderState(){
    if(recordingSwitch){
        // recording
        ofSetColor(ofColor::red);
        ofVec2f circlePosition(ofGetWidth() - 20, 20);
        float circleRadius = 5;
        ofDrawCircle(circlePosition, circleRadius);
        ofSetColor(255);
    }else{
        if (videoRecorder.isPaused()){
            // pause
            ofRectangle rectangleLeftPosition(ofGetWidth() - 23, 16, 2, 8);
            ofRectangle rectangleRightPosition(ofGetWidth() - 20, 16, 2, 8);
            ofDrawRectangle(rectangleLeftPosition);
            ofDrawRectangle(rectangleRightPosition);
        } else {
            // stop
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key) {
        case 'h': // hide or show gui
            showGui = !showGui;
            showGui ? ofShowCursor() : ofHideCursor();
            break;
        case 'l': // load the gui setting
            panel.loadFromFile(SETTING_FILE_NAME);
            break;
        case 'r': // rec start/stop
            recordingSwitch = !recordingSwitch;
            break;
        case 's': // save the gui setting
            panel.saveToFile(SETTING_FILE_NAME);
            break;
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
