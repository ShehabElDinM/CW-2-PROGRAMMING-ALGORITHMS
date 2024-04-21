#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>
#include <fstream>
#include <pthread.h>

using namespace std;

void log(const string& message) {
    cout << message << endl;
}

string Encrypt(const string& message, int shift) {
    string result = "";
    for (char c : message) {
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            result += (c - base + shift) % 26 + base;
        } else {
            result += c;
        }
    }
    return result;
}

string Decrypt(const string& message, int shift) {
    return Encrypt(message, 26 - shift);
}