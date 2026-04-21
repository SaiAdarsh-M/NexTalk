#include "ImageMessage.h"
#include <iostream>
using namespace std;

ImageMessage::ImageMessage(string id, string sender, string time, string path)
    : Message(id, sender, time) {
    this->imagePath = path;
}

void ImageMessage::display() {
    cout << sender << " sent an image: " << imagePath << endl;
}