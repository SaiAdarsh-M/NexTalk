#include <iostream>
#include "src/models/Chat.h"
#include "src/models/TextMessage.h"
#include "src/models/ImageMessage.h"
#include "src/models/GroupChat.h"



using namespace std;

int main() {
    Chat chat("chat1");

    chat.addParticipant("Sai");
    chat.addParticipant("Rahul");

    int choice;
    string sender, text;

    while (true) {
        cout << "\n1. Send Message\n2. Show Chat\n3. Undo\n4. Exit\n";
        cin >> choice;

        if (choice == 1) {
            cout << "Enter sender: ";
            cin >> sender;

            cout << "Enter message: ";
            cin.ignore();
            getline(cin, text);

            chat.sendMessage(new TextMessage("id", sender, "time", text));
        }
        else if (choice == 2) {
            chat.displayChat();
        }
        else if (choice == 3) {
            chat.undo();
        }
        else {
            break;
        }
    }

    return 0;
}