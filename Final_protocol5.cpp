
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

void simulateRetransmissions(vector<Frame>& frames, int& base, int windowSize) {
    // Simulate retransmissions for unacknowledged frames
    for (int i = base; i < base + windowSize && i < frames.size(); ++i) {
        if (!frames[i].ack && !frames[i].retransmitted) {
            // Simulate retransmission
            cout << "Retransmitting frame " << frames[i].id << endl;
            frames[i].retransmitted = true;
            frames[i].ack = false; // Reset ack for retransmission
        }
    }
}

void runTestCase(int windowSize, int totalFrames, int efficiency) {
    vector<Frame> frames;
    for (int i = 1; i <= totalFrames; ++i) {
        frames.push_back(Frame(i));  // Ensure frame IDs start from 1
    }

    int base = 0;
    int nextSeqNum = 0;

    while (base < totalFrames) {
        sendFrames(frames, base, nextSeqNum, windowSize);
        receiveAcks(frames, base, windowSize, efficiency);
        simulateRetransmissions(frames, base, windowSize);
    }

    cout << "All frames sent and acknowledged." << endl;
}

int main() {
    int choice;
    cout << "Select an option:\n";
    cout << "1. Run predefined test case\n";
    cout << "2. Enter custom parameters\n";
    cout << "Enter your choice: ";
    cin >> choice;

    if (choice == 1) {
        int testCase;
        cout << "Select a test case (1-8): ";
        cin >> testCase;

        switch (testCase) {
        case 1:
            runTestCase(4, 10, 100); // High Link Efficiency (100%)
            break;
        case 2:
            runTestCase(4, 10, 0);   // No ACKs, low efficiency (0%)
            break;
        case 3:
            runTestCase(4, 10, 50);  // Medium Link Efficiency (50%)
            break;
        case 4:
            runTestCase(6, 10, 70);  // Larger Window Size (6)
            break;



