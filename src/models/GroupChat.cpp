#include "GroupChat.h"
#include <iostream>
using namespace std;

GroupChat::GroupChat(string id, string name) : Chat(id) {
    groupName = name;
}

void GroupChat::displayGroupInfo() {
    cout << "Group: " << groupName << endl;
}