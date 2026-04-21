
#include "VideoMessage.h"
#include <iostream>
using namespace std;

VideoMessage::VideoMessage(string id, string sender, string time, string path)
    : Message(id, sender, time) {
    this->videoPath = path;
}

void VideoMessage::display() {
    cout << sender << " sent a video: " << videoPath << endl;
}

