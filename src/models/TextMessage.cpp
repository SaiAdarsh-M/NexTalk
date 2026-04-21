#include "TextMessage.h"
#include <iostream>
using namespace std;

TextMessage::TextMessage(string id, string sender, string time, string text)
    : Message(id, sender, time) {
    this->text = text;
}

void TextMessage::display() {
    cout << sender << ": " << text << endl;
}

string TextMessage::getText() {
    return text;
}