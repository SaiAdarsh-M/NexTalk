#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include <string>
#include <utility>
#include <vector>

using namespace std;

class Database {
 private:
  sqlite3* db;

 public:
  Database();
  ~Database();

  void insertMessage(string id, string sender, string receiver, string content);
  void setActiveChat(string user, string chat);
  vector<vector<string>> getMessages();  // ✅ UPDATED
  vector<string> getUsers();
  bool isMessageOwner(string id, string user);
  void markAllSeen(string user);
  void deleteForMe(string messageId, string user);
  void deleteForEveryone(string messageId);
  void markSeen(string id);
  void markAllSeen(string sender, string receiver);
  void insertUser(string name);
  void insertUserWithPassword(string user, string pass);
  bool validateUser(string user, string pass);
  void deleteUser(const std::string& user);
  void deleteAllMessagesOfUser(const std::string& user);
  void updatePassword(string user, string newPass);
  sqlite3* getDB();
};

#endif