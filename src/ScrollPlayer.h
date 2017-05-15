#pragma once

class ScrollPlayer{
public:
    vector<shared_ptr<MovingVideoPlayer>> players;
    int rightViewId;
    ofVec2f rightViewPosition;
    ofVec2f movingSpeed;
};
