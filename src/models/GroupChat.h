#ifndef GROUPCHAT_H
#define GROUPCHAT_H

#include "Chat.h"

class GroupChat : public Chat {
private:
    string groupName;

public:
    GroupChat(string id, string name);

    void displayGroupInfo();
};

#endif