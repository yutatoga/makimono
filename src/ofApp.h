#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#define SPEED_MINIMUM_X 0
#define SPEED_MINIMUM_Y -30

#define SPEED_MAXIMUM_X 30
#define SPEED_MAXIMUM_Y 30

#define FRAME_RATE 60

#define DEBUG_PLAYER_WIDTH 128
#define DEBUG_PLAYER_HEIGHT 72

class ScrollPlayer{
public:
    vector<shared_ptr<ofVideoPlayer>> players;
    int rightViewId;
    ofVec2f rightViewPosition;
    ofVec2f movingSpeed;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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
		
    void movingSpeedChanged(ofVec2f &movingSpeed);
    void borderPositionChanged(float &borderPosition);
    
    // debug
    void drawAllPlayers();
    ofVec2f getPlayerPosition(int playerIndex);
    void drawBorder(ofColor lineColor);
    
    ScrollPlayer scrollPlayer;
    
    // gui
    bool showGui;
    ofxPanel panel;
    ofParameter<ofVec2f> movingSpeed;
    ofParameter<string> fps;
    ofParameter<bool> enableDrawAllPlayers;
    ofParameter<bool> enableDrawBorder;
    ofParameter<float> windowSize;
    ofParameter<float> borderPosition;
    ofxLabel shortCutInfo;
};