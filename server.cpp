
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "crow/crow.h"
#include "crow/middlewares/cors.h"
#include "src/models/Chat.h"
#include "src/models/TextMessage.h"
#include "src/utils/Database.h"

using namespace std;
unordered_map<string, vector<crow::websocket::connection*>> userConnections;
unordered_set<string> onlineUsers;
vector<crow::websocket::connection*> clients;

Chat chat("chat1");

int main() {
  Database db;

  crow::App<crow::CORSHandler> app;

  auto& cors = app.get_middleware<crow::CORSHandler>();
  cors.global().headers("Content-Type").methods("POST"_method, "GET"_method).origin("*");

  // 🔌 WebSocket
  CROW_WEBSOCKET_ROUTE(app, "/ws")
      .onopen([&](crow::websocket::connection& conn) {
        cout << "🔌 WebSocket connected\n";
        clients.push_back(&conn);
      })

      .onclose([&](crow::websocket::connection& conn, const string& reason, uint16_t code) {
        cout << "❌ WebSocket disconnected\n";

        // ✅ remove from clients
        clients.erase(remove(clients.begin(), clients.end(), &conn), clients.end());

        // 🔥 remove from userConnections + handle offline
        for (auto it = userConnections.begin(); it != userConnections.end();) {
          string user = it->first;  // ✅ SAVE USER FIRST
          auto& conns = it->second;

          conns.erase(remove(conns.begin(), conns.end(), &conn), conns.end());

          if (conns.empty()) {
            it = userConnections.erase(it);

            // 🔥 mark offline
            onlineUsers.erase(user);

            // 🔥 broadcast offline
            crow::json::wvalue res;
            res["type"] = "offline";
            res["user"] = user;

            for (auto client : clients) {
              client->send_text(res.dump());
            }

          } else {
            ++it;
          }
        }
      })

      .onmessage([&](crow::websocket::connection& conn, const string& data, bool is_binary) {
        auto body = crow::json::load(data);

        if (!body) {
          cout << "❌ JSON parse failed\n";
          return;
        }

        // ✅ SAFE sender extraction
        string sender = "";

        if (body.has("sender")) {
          sender = string(body["sender"].s());
        } else if (body.has("user")) {
          sender = string(body["user"].s());  // 🔥 IMPORTANT FIX
        }

        string receiver = "";
        if (body.has("receiver")) {
          receiver = string(body["receiver"].s());
        }

        // ✅ STORE CONNECTION
        if (!sender.empty()) {
          auto& conns = userConnections[sender];

          // avoid duplicate insert
          if (find(conns.begin(), conns.end(), &conn) == conns.end()) {
            conns.push_back(&conn);
          }
        }

        // existing
        string type = body["type"].s();
        cout << "[WS] Type: " << type << endl;

        if (type == "join") {
          if (!sender.empty()) {
            onlineUsers.insert(sender);

            // 🔥 1. SEND FULL LIST TO THIS USER ONLY
            crow::json::wvalue listMsg;
            listMsg["type"] = "online_list";

            int i = 0;
            for (const auto& u : onlineUsers) {
              listMsg["users"][i++] = u;
            }

            conn.send_text(listMsg.dump());  // ✅ IMPORTANT

            // 🔥 2. BROADCAST THIS USER ONLINE
            crow::json::wvalue res;
            res["type"] = "online";
            res["user"] = sender;

            for (auto client : clients) {
              client->send_text(res.dump());
            }
          }
        }

        // 🔹 TYPING
        else if (type == "typing") {
          cout << "⌨️ " << sender << " typing to " << receiver << endl;

          // ✅ send ONLY to receiver
          crow::json::wvalue typingMsg;
          typingMsg["type"] = "typing";
          typingMsg["sender"] = sender;
          typingMsg["receiver"] = receiver;

          if (userConnections.count(receiver)) {
            for (auto c : userConnections[receiver]) {
              c->send_text(typingMsg.dump());
            }
          }
        }

        // 🔹 MESSAGE
        else if (type == "message") {
          string id = body["id"].s();
          string text = body["text"].s();
          string time = body["time"].s();
          receiver = body["receiver"].s();  // ✅ FIXED

          // ✅ NOW safe
          db.insertUser(sender);
          db.insertUser(receiver);

          // send new_user only once per message
          crow::json::wvalue userMsg1;
          userMsg1["type"] = "new_user";
          userMsg1["user"] = sender;

          crow::json::wvalue userMsg2;
          userMsg2["type"] = "new_user";
          userMsg2["user"] = receiver;

          for (auto client : clients) {
            client->send_text(userMsg1.dump());
            client->send_text(userMsg2.dump());
          }

          cout << "📨 MESSAGE from " << sender << " to " << receiver << ": " << text << endl;

          db.insertMessage(id, sender, receiver, text);

          crow::json::wvalue msg;
          msg["type"] = "message";
          msg["id"] = id;
          msg["sender"] = sender;
          msg["text"] = text;
          msg["time"] = time;
          msg["receiver"] = receiver;

          // ✅ SENT
          msg["status"] = "sent";
          if (userConnections.count(sender)) {
            for (auto c : userConnections[sender]) {
              c->send_text(msg.dump());
            }
          }

          // ✅ DELIVERED
          msg["status"] = "delivered";

          if (userConnections.count(sender)) {
            for (auto c : userConnections[sender]) {
              c->send_text(msg.dump());
            }
          }

          if (userConnections.count(receiver)) {
            for (auto c : userConnections[receiver]) {
              c->send_text(msg.dump());  // ✅ correct
            }
          }
        }

        // 🔹 SEEN (single message)
        else if (type == "seen") {
          string messageId = body["id"].s();
          string sender = body["sender"].s();      // 👈 who saw
          string receiver = body["receiver"].s();  // 👈 original sender

          cout << "👀 Message seen: " << messageId << " by " << sender << endl;

          db.markSeen(messageId);  // ✅ update DB

          crow::json::wvalue seenMsg;
          seenMsg["type"] = "seen";
          seenMsg["id"] = messageId;

          // ✅ send ONLY to original sender
          if (userConnections.count(receiver)) {
            for (auto c : userConnections[receiver]) {
              c->send_text(seenMsg.dump());
            }
          }
        }

        // 🔹 SEEN ALL
        else if (type == "seen_all") {
          string receiver = body["receiver"].s();

          cout << "👀 All messages seen by: " << sender << endl;
          db.markAllSeen(sender, receiver);
          crow::json::wvalue seenMsg;
          seenMsg["type"] = "seen_all";
          seenMsg["sender"] = sender;

          // ✅ send only to other user
          if (userConnections.count(receiver)) {
            for (auto c : userConnections[receiver]) {
              c->send_text(seenMsg.dump());
            }
          }
        }

        // 🔹 DELETE (hard delete - optional)
        else if (type == "delete") {
          string messageId = body["id"].s();

          cout << "🗑️ Hard delete message: " << messageId << endl;

          string query = "DELETE FROM messages WHERE id='" + messageId + "'";
          sqlite3_exec(db.getDB(), query.c_str(), 0, 0, 0);

          crow::json::wvalue delMsg;
          delMsg["type"] = "delete";
          delMsg["id"] = messageId;

          // ✅ send only to sender + receiver
          if (userConnections.count(sender)) {
            for (auto c : userConnections[sender]) {
              c->send_text(delMsg.dump());
            }
          }
          if (userConnections.count(receiver)) {
            for (auto c : userConnections[receiver]) {
              c->send_text(delMsg.dump());
            }
          }
        }

        // 🔹 DELETE FOR ME
        else if (type == "delete_for_me") {
          string messageId = body["id"].s();
          string user = body["user"].s();

          cout << "🙈 Delete for me by " << user << " (msg: " << messageId << ")\n";

          db.deleteForMe(messageId, user);

          crow::json::wvalue res;
          res["type"] = "delete_for_me";
          res["id"] = messageId;
          res["user"] = user;

          // ✅ send only to that user
          if (userConnections.count(user)) {
            for (auto c : userConnections[user]) {
              c->send_text(res.dump());
            }
          }
        }

        // 🔹 DELETE FOR EVERYONE (secure)
        else if (type == "delete_for_everyone") {
          string messageId = body["id"].s();
          string user = body["user"].s();
          string receiver = body["receiver"].s();

          cout << "🗑️ Delete request by " << user << " for message " << messageId << endl;

          if (!db.isMessageOwner(messageId, user)) {
            cout << "❌ Unauthorized delete attempt\n";
            return;
          }

          db.deleteForEveryone(messageId);

          cout << "✅ Message deleted for everyone\n";

          crow::json::wvalue res;
          res["type"] = "delete";
          res["id"] = messageId;

          // ✅ send to both users
          if (userConnections.count(user)) {
            for (auto c : userConnections[user]) {
              c->send_text(res.dump());
            }
          }

          if (userConnections.count(receiver)) {
            for (auto c : userConnections[receiver]) {
              c->send_text(res.dump());
            }
          }
        }
      });
  // 🔹 SEND API (fallback)
  CROW_ROUTE(app, "/send").methods("POST"_method)([&](const crow::request& req) {
    auto body = crow::json::load(req.body);

    string sender = body["sender"].s();
    string text = body["text"].s();
    string id = to_string(time(0));
    string receiver = body["receiver"].s();

    db.insertMessage(id, sender, receiver, text);

    cout << "📨 API message from " << sender << ": " << text << endl;

    return crow::response(200, "Message Sent");
  });

  // 🔹 GET MESSAGES
  CROW_ROUTE(app, "/messages")
  ([&]() {
    cout << "➡️ Enter /messages route" << endl;

    auto messages = db.getMessages();
    cout << "➡️ Messages fetched: " << messages.size() << endl;

    crow::json::wvalue result;

    int index = 0;

    for (auto& row : messages) {
      if (row.size() < 7) continue;

      crow::json::wvalue obj;

      obj["id"] = crow::json::wvalue(row[0]);
      obj["sender"] = crow::json::wvalue(row[1]);
      obj["receiver"] = crow::json::wvalue(row[2]);
      obj["text"] = crow::json::wvalue(row[3]);
      obj["time"] = crow::json::wvalue(row[4]);
      obj["status"] = crow::json::wvalue(row[5]);
      obj["deleted_for"] = crow::json::wvalue(row[6]);

      result[index++] = std::move(obj);
    }

    cout << "➡️ Sending response" << endl;
    return result;
  });
  CROW_ROUTE(app, "/setActiveChat").methods("POST"_method)([&](const crow::request& req) {
    auto body = crow::json::load(req.body);

    string user = body["user"].s();
    string chat = body["chat"].s();

    db.setActiveChat(user, chat);

    return crow::response(200, "Saved");
  });

  CROW_ROUTE(app, "/status")
  ([&](const crow::request& req) {
    auto userParam = req.url_params.get("user");

    crow::json::wvalue res;

    if (!userParam) {
      res["error"] = "Missing user";
      return res;
    }

    string user = userParam;

    res["online"] = userConnections.count(user) && !userConnections[user].empty();

    return res;
  });
  CROW_ROUTE(app, "/users")
  ([&]() {
    crow::json::wvalue result;

    auto users = db.getUsers();

    crow::json::wvalue::list userList;  // 🔥 USE LIST (SAFE)

    for (auto& user : users) {
      userList.push_back(user);
    }

    result = std::move(userList);  // 🔥 assign list

    return result;
  });
  CROW_ROUTE(app, "/register").methods("POST"_method)([&](const crow::request& req) {
    auto body = crow::json::load(req.body);

    string user = body["username"].s();
    string pass = body["password"].s();

    db.insertUserWithPassword(user, pass);

    return crow::response(200, "Registered");
  });
  CROW_ROUTE(app, "/login").methods("POST"_method)([&](const crow::request& req) {
    auto body = crow::json::load(req.body);

    string user = body["username"].s();
    string pass = body["password"].s();

    if (db.validateUser(user, pass)) {
      return crow::response(200, "OK");
    }

    return crow::response(401, "Invalid");
  });
  CROW_ROUTE(app, "/deleteAccount").methods("POST"_method)([&](const crow::request& req) {
    auto body = crow::json::load(req.body);

    string user = body["username"].s();

    cout << "🗑️ Deleting account: " << user << endl;
    // 🔥 delete messages
    db.deleteAllMessagesOfUser(user);

    // 🔥 delete user from DB
    db.deleteUser(user);

    // 🔥 remove from onlineUsers
    onlineUsers.erase(user);

    // 🔥 remove connections
    userConnections.erase(user);

    // 🔥 broadcast offline
    crow::json::wvalue res;
    res["type"] = "user_deleted";

    res["username"] = user;

    for (auto client : clients) {
      client->send_text(res.dump());
    }

    return crow::response(200, "Account deleted");
  });
  CROW_ROUTE(app, "/changePassword").methods("POST"_method)([&](const crow::request& req) {
    auto body = crow::json::load(req.body);

    string user = body["username"].s();
    string oldPass = body["oldPassword"].s();
    string newPass = body["newPassword"].s();

    // 🔒 validate old password
    if (!db.validateUser(user, oldPass)) {
      return crow::response(401, "Wrong password");
    }

    // 🔥 update password
    db.updatePassword(user, newPass);

    cout << "🔑 Password changed for: " << user << endl;

    return crow::response(200, "Password updated");
  });

  const char* port = getenv("PORT");
  int p = port ? std::stoi(port) : 18080;

  app.port(p).run();
}