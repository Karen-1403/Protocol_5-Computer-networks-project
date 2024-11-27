
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

class Frame {
public:
    int id;
    bool ack;
    bool retransmitted;

    Frame(int id) : id(id), ack(false), retransmitted(false) {}
};

void sendFrames(vector<Frame>& frames, int& base, int& nextSeqNum, int windowSize) {
    for (int i = 0; i < windowSize && nextSeqNum < frames.size(); ++i) {
        if (nextSeqNum < base + windowSize) {
            cout << "Sending frame " << frames[nextSeqNum].id << endl;
            frames[nextSeqNum].ack = false;
            frames[nextSeqNum].retransmitted = false;
            nextSeqNum++;
        }
    }
}

void receiveAcks(vector<Frame>& frames, int& base, int windowSize, int efficiency) {
    srand(time(NULL)); // Seed the random number generator with the current time
    for (int i = base; i < base + windowSize && i < frames.size(); ++i) {
        int randNum = rand() % 100;  // Generate a random number between 0 and 99
        if (randNum < efficiency) { // Simulate ACK reception based on efficiency percentage
            cout << "ACK received for frame " << frames[i].id << endl;
            frames[i].ack = true;
        }
        else {
            cout << "ACK not received for frame " << frames[i].id << endl;
        }
    }

    while (base < frames.size() && frames[base].ack) {
        base++;
    }
}






