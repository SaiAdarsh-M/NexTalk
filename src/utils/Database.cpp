#include "Database.h"

#include <iostream>
#include <utility>
#include <vector>

static int messageCallback(void* data, int argc, char** argv, char**) {
  auto* msgs = static_cast<vector<vector<string>>*>(data);

  vector<string> row;

  for (int i = 0; i < argc; i++) {
    if (argv && argv[i]) {
      row.push_back(string(argv[i]));
    } else {
      row.push_back("");
    }
  }

  while (row.size() < 7) {
    row.push_back("");
  }

  msgs->push_back(row);
  return 0;
}
static int ownerCallback(void* data, int argc, char** argv, char**) {
  if (argc > 0 && argv && argv[0]) {
    *(string*)data = string(argv[0]);
  }
  return 0;
}

Database::Database() {
  if (sqlite3_open("chat.db", &db)) {
    cout << "Error opening database\n";
  }
  cout << "🔥 Constructor running\n";
  // ✅ CREATE TABLE IF NOT EXISTS
  string sql =
      "CREATE TABLE IF NOT EXISTS messages ("
      "id TEXT PRIMARY KEY, "
      "sender TEXT, "
      "receiver TEXT, "
      "content TEXT, "
      "timestamp TEXT, "
      "status TEXT, "
      "deleted_for TEXT, "
      "is_deleted INTEGER DEFAULT 0);";

  char* errMsg;
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg)) {
    cout << "Table creation failed\n";
  } else {
    cout << "Table ready" << endl;
  }

  string sql2 =
      "CREATE TABLE IF NOT EXISTS user_sessions ("
      "user TEXT PRIMARY KEY, "
      "active_chat TEXT);";

  sqlite3_exec(db, sql2.c_str(), 0, 0, 0);

  string sqlUsers =
      "CREATE TABLE IF NOT EXISTS users ("
      "name TEXT PRIMARY KEY, "
      "password TEXT);";

  sqlite3_exec(db, sqlUsers.c_str(), 0, 0, 0);
}
Database::~Database() {
  sqlite3_close(db);
}

bool Database::isMessageOwner(string id, string user) {
  string query = "SELECT sender FROM messages WHERE id='" + id + "';";

  string owner = "";

  char* errMsg = nullptr;

  if (sqlite3_exec(db, query.c_str(), ownerCallback, &owner, &errMsg)) {
    cout << "❌ Error in isMessageOwner: " << errMsg << endl;
    sqlite3_free(errMsg);
    return false;
  }

  return !owner.empty() && owner == user;
}
void Database::insertMessage(string id, string sender, string receiver, string content) {
  string sql =
      "INSERT INTO messages (id, sender, receiver, content, timestamp, status, deleted_for, "
      "is_deleted) "
      "VALUES ('" +
      id + "', '" + sender + "', '" + receiver + "', '" + content +
      "', datetime('now'), 'sent', '', 0);";
  char* errMsg;
  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg)) {
    cout << "Insert failed: " << errMsg << endl;
  }
}
void Database::setActiveChat(string user, string chat) {
  string sql = "INSERT INTO user_sessions (user, active_chat) VALUES ('" + user + "', '" + chat +
               "') ON CONFLICT(user) DO UPDATE SET active_chat='" + chat + "';";

  sqlite3_exec(db, sql.c_str(), 0, 0, 0);
}
void Database::markSeen(string id) {
  string sql = "UPDATE messages SET status='seen' WHERE id='" + id + "';";
  sqlite3_exec(db, sql.c_str(), 0, 0, 0);
}
void Database::markAllSeen(string sender, string receiver) {
  string sql =
      "UPDATE messages SET status='seen' "
      "WHERE sender='" +
      receiver + "' AND receiver='" + sender + "';";

  char* errMsg;

  if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg)) {
    cout << "Failed to update seen_all\n";
  } else {
    cout << "All messages seen between " << sender << " and " << receiver << endl;
  }
}

vector<vector<string>> Database::getMessages() {
  vector<vector<string>> messages;

  string sql =
      "SELECT id, sender, receiver, content, timestamp, status, deleted_for FROM messages;";

  sqlite3_exec(db, sql.c_str(),
               messageCallback,  // 🔥 THIS IS THE FIX
               &messages, nullptr);

  return messages;
}
vector<string> Database::getUsers() {
  vector<string> users;

  string sql = "SELECT name FROM users;";

  char* errMsg = nullptr;

  auto callback = [](void* data, int argc, char** argv, char**) {
    auto* list = static_cast<vector<string>*>(data);

    if (argc > 0 && argv && argv[0]) {
      list->push_back(string(argv[0]));
    }

    return 0;
  };

  if (sqlite3_exec(db, sql.c_str(), callback, &users, &errMsg)) {
    cout << "❌ Error fetching users: " << errMsg << endl;
    sqlite3_free(errMsg);  // 🔥 IMPORTANT (avoid memory leak)
  } else {
    cout << "✅ Users fetched: " << users.size() << endl;
  }

  return users;
}

void Database::deleteForMe(string messageId, string user) {
  string query =
      "UPDATE messages SET deleted_for = "
      "CASE "
      "WHEN deleted_for IS NULL OR deleted_for = '' THEN '" +
      user +
      "' "
      "ELSE deleted_for || ',' || '" +
      user +
      "' "
      "END "
      "WHERE id='" +
      messageId + "';";

  sqlite3_exec(db, query.c_str(), 0, 0, 0);
}
void Database::deleteForEveryone(string messageId) {
  string query =
      "UPDATE messages SET "
      "content='This message was deleted', "
      "is_deleted=1 "
      "WHERE id='" +
      messageId + "';";

  char* errMsg;

  if (sqlite3_exec(db, query.c_str(), 0, 0, &errMsg)) {
    cout << "❌ Failed to delete message\n";
  } else {
    cout << "✅ Message marked as deleted\n";
  }
}
void Database::insertUser(string name) {
  string sql = "INSERT OR IGNORE INTO users (name) VALUES ('" + name + "');";
  sqlite3_exec(db, sql.c_str(), 0, 0, 0);
}
sqlite3* Database::getDB() {
  return db;
}
void Database::insertUserWithPassword(string user, string pass) {
  string sql =
      "INSERT OR IGNORE INTO users (name, password) VALUES ('" + user + "','" + pass + "');";
  sqlite3_exec(db, sql.c_str(), 0, 0, 0);
}
bool Database::validateUser(string user, string pass) {
  string sql = "SELECT password FROM users WHERE name='" + user + "';";
  string dbPass = "";

  sqlite3_exec(
      db, sql.c_str(),
      [](void* data, int argc, char** argv, char**) {
        if (argc > 0 && argv[0]) {
          *(string*)data = argv[0];
        }
        return 0;
      },
      &dbPass, nullptr);

  return dbPass == pass;
}
void Database::deleteUser(const string& user) {
  string query = "DELETE FROM users WHERE name='" + user + "';";
  sqlite3_exec(db, query.c_str(), 0, 0, 0);
}
void Database::deleteAllMessagesOfUser(const string& user) {
  string query = "DELETE FROM messages WHERE sender='" + user + "' OR receiver='" + user + "';";

  sqlite3_exec(db, query.c_str(), 0, 0, 0);
}
void Database::updatePassword(string user, string newPass) {
  string query = "UPDATE users SET password='" + newPass + "' WHERE name='" + user + "';";

  char* errMsg;

  if (sqlite3_exec(db, query.c_str(), 0, 0, &errMsg)) {
    cout << "❌ Failed to update password\n";
  } else {
    cout << "✅ Password updated in DB\n";
  }
}