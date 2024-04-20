#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;

using namespace std;

constexpr int NUM_USERS = 100;

struct User_Info {
    int socket;
    bool authenticated;
};

User_Info users[NUM_USERS];

int num_clients = 0;

void Log(const string& message) {
    cout << message << endl;
}
