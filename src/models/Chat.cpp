#include "Chat.h"
#include "TextMessage.h"
#include <iostream>
using namespace std;

Chat::Chat(string id) {
    this->chatId = id;
}

void Chat::addParticipant(string user) {
    participants.push_back(user);
}

void Chat::sendMessage(Message* msg) {
     saveState();
    messages.push_back(msg);

    // mark delivered for all except sender
    for (auto user : participants) {
        if (user != msg->getSender()) {
            msg->setStatus(user, DELIVERED);
        }
    }
}

void Chat::displayChat() {
    cout << "---- Chat ----" << endl;

    for (auto msg : messages) {

        // if deleted for everyone
        if (msg->isDeletedForUser("ALL")) {
            cout << "[Message Deleted for Everyone]" << endl;
            continue;
        }

        msg->display();

        for (auto user : participants) {
            cout << " [" << user << ": " << msg->getStatus(user) << "]";
        }

        cout << endl;
    }
}

void Chat::markAsSeen(string user) {
    saveState();
    for (auto msg : messages) {
        msg->setStatus(user, SEEN);
    }
}

void Chat::deleteForEveryone(string messageId) {
    saveState();
    for (auto msg : messages) {
        if (msg->getId() == messageId) {
            msg->deleteForEveryone();
        }
    }
}

void Chat::deleteForUser(string messageId, string user) {
    saveState();
    for (auto msg : messages) {
        if (msg->getId() == messageId) {
            msg->deleteForUser(user);
        }
    }
}

void Chat::saveState() {
    history.push(ChatState(messages));
}

void Chat::undo() {
    if (!history.empty()) {
        messages = history.top().messages;
        history.pop();
    } else {
        cout << "Nothing to undo!" << endl;
    }
}
vector<Message*> Chat::searchByKeyword(string keyword) {
    vector<Message*> result;

    for (auto msg : messages) {
        TextMessage* txt = dynamic_cast<TextMessage*>(msg);

        if (txt != nullptr) {
            if (txt->getText().find(keyword) != string::npos) {
                result.push_back(msg);
            }
        }
    }

    return result;
}

vector<Message*> Chat::searchByUser(string user) {
    vector<Message*> result;

    for (auto msg : messages) {
        if (msg->getSender() == user) {
            result.push_back(msg);
        }
    }

    return result;
}

vector<Message*>& Chat::getMessages() {
    return messages;
}