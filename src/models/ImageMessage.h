#ifndef IMAGEMESSAGE_H
#define IMAGEMESSAGE_H

#include "Message.h"

class ImageMessage : public Message {
private:
    string imagePath;

public:
    ImageMessage(string id, string sender, string time, string path);

    void display() override;
};

#endif