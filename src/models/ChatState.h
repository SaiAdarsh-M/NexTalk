#ifndef CHATSTATE_H
#define CHATSTATE_H

#include <vector>
#include "Message.h"

using namespace std;

class ChatState {
public:
    vector<Message*> messages;

    ChatState(vector<Message*> msgs) {
        messages = msgs;
    }
};

#endif