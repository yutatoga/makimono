#pragma once

class MovingVideoPlayer: public ofVideoPlayer{
public:
    void setVolume(float volume){
        isMuted = (volume == 0);
        ofVideoPlayer::setVolume(volume);
    }
    
    bool isMuted;
};
