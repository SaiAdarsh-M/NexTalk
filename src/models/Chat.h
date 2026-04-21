#ifndef CHAT_H
#define CHAT_H

#include <stack>
#include "ChatState.h"
#include <vector>
#include <string>
#include "Message.h"

using namespace std;

class Chat {
private:
    string chatId;
    vector<string> participants;
    vector<Message*> messages;
    stack<ChatState> history;   // ✅ ADD HERE

public:
    Chat(string id);

    void addParticipant(string user);
    void sendMessage(Message* msg);

    void displayChat();

    void markAsSeen(string user);

    void deleteForEveryone(string messageId);
    
    void deleteForUser(string messageId, string user);
    
    void saveState();   // ✅ ADD HERE
    void undo();        // ✅ ADD HERE
    
    vector<Message*> searchByKeyword(string keyword);
    vector<Message*> searchByUser(string user);

    vector<Message*>& getMessages(); // for advanced features later
};

#endif