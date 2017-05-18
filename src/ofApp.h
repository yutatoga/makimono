#pragma once

#include "ofMain.h"

// addons
#include "ofxGui.h"
#include "ofxVideoRecorder.h"

// classes
#include "MovingVideoPlayer.h"
#include "ScrollPlayer.h"
#include "MovingVideoPlayer.h"

#define FRAME_RATE 60

// gui
#define SPEED_MINIMUM_X 0
#define SPEED_MINIMUM_Y -30

#define SPEED_MAXIMUM_X 30
#define SPEED_MAXIMUM_Y 30

#define GUI_POSITION_X 10
#define GUI_POSITION_Y 10
#define SETTING_FILE_NAME "settings.xml"

// video grabber
#define VIDEO_GRABBER_FRAME_RATE 30

// video recorder
#define VIDEO_FOLDER_NAME "movies"
#define VIDEO_ERROR_FOLDER_NAME "error"
#define VIDEO_FILE_NAME_HEADER "testMovie_"
#define VIDEO_FILE_NAME_EXTENSION ".mov"
#define VIDEO_DEVICE_ID 0
#define VIDEO_CODEC "mpeg4"
#define VIDEO_BITRATE "800k"
#define VIDEO_RECORDER_FRAME_RATE 30
#define VIDEO_MINIMUM_LENGTH 3
#define AUDIO_CODEC "mp3"
#define AUDIO_BITRATE "192k"


// sound stream
#define SOUND_DEVICE_ID 0
#define SOUND_SAMPLE_RATES_INDEX 0
#define SOUND_BUFFER_SIZE 256
#define SOUND_NUMBER_OF_BUFFER 4

// debug
#define DEBUG_PLAYER_WIDTH 128
#define DEBUG_PLAYER_HEIGHT 72

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
        void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void audioIn(float * input, int bufferSize, int nChannels);
	
    // listener
    void movingSpeedChanged(ofVec2f &movingSpeed);
    void borderPositionChanged(float &borderPosition);
    void recordingSwitchChanged(bool &recordingSwitch);
    void recordingComplete(ofxVideoRecorderOutputFileCompleteEventArgs& args);
    
    // debug
    void drawAllPlayers();
    ofVec2f getPlayerPosition(int playerIndex);
    void drawBorder(ofColor lineColor);
    void printVideoDeviceList(ofVideoGrabber videoGrabber);
    void printSoundDeviceList(ofSoundStream soundStream);
    void drawVideoRecorderState();
    
    ScrollPlayer scrollPlayer;
    ofVideoGrabber videoGrabber;
    ofxVideoRecorder videoRecorder;
    ofSoundStream soundStream;
    ofSoundDevice soundDevice;
    bool loading;
    string loadingFileName;
    
    // gui
    bool showGui;
    ofxPanel panel;
    ofParameter<ofVec2f> movingSpeed;
    ofParameter<string> fps;
    ofParameter<bool> enableDrawAllPlayers;
    ofParameter<bool> enableDrawBorder;
    ofParameter<bool> enableDrawVideoGrabber;
    ofParameter<bool> enableDrawVideoRecorderState;
    ofParameter<bool> enableFullPlay;
    ofParameter<float> windowSize;
    ofParameter<float> borderPosition;
    ofParameter<bool> recordingSwitch;
    ofxLabel shortCutInfo;
};
