#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <string>
#include <map>
#include <set>
using namespace std;

// Message status
enum Status { SENT, DELIVERED, SEEN };

class Message {
protected:
    string messageId;
    string sender;
    string timestamp;

    map<string, Status> userStatus;   // each user has status
    bool deletedForEveryone;
    set<string> deletedForUsers;

public:
    Message(string id, string sender, string time);

    virtual void display() = 0; // polymorphism (IMPORTANT)

    void setStatus(string user, Status status);
    Status getStatus(string user);

    void deleteForEveryone();
    void deleteForUser(string user);
    bool isDeletedForUser(string user);

    string getSender();
    string getId();

    virtual ~Message() {}
};

#endif