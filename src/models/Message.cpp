#include "Message.h"

Message::Message(string id, string sender, string time) {
    this->messageId = id;
    this->sender = sender;
    this->timestamp = time;
    this->deletedForEveryone = false;
}

void Message::setStatus(string user, Status status) {
    userStatus[user] = status;
}

Status Message::getStatus(string user) {
    if (userStatus.find(user) != userStatus.end())
        return userStatus[user];
    return SENT; // default if not found
}

void Message::deleteForEveryone() {
    deletedForEveryone = true;
}

void Message::deleteForUser(string user) {
    deletedForUsers.insert(user);
}

bool Message::isDeletedForUser(string user) {
    if (deletedForEveryone) return true;
    return deletedForUsers.count(user);
}

string Message::getSender() {
    return sender;
}

string Message::getId() {
    return messageId;
}