#ifndef TEXTMESSAGE_H
#define TEXTMESSAGE_H

#include "Message.h"

class TextMessage : public Message {
private:
    string text;

public:
    TextMessage(string id, string sender, string time, string text);

    void display() override;

    string getText();
};

#endif