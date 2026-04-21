
#ifndef VIDEOMESSAGE_H
#define VIDEOMESSAGE_H

#include "Message.h"

class VideoMessage : public Message {
private:
    string videoPath;

public:
    VideoMessage(string id, string sender, string time, string path);

    void display() override;
};

#endif