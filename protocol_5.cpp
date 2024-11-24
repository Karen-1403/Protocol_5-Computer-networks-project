#include <iostream>
#include <ctime>
#include <cstdlib>

using namespace std;

// Function to simulate the protocol
void protocol5(int windowSize, int numberOfFrames, int wireEfficiency) {
    srand(time(NULL));
    int lastSuccessfulFrame = 0;

    // Initial sending of frames within the window size
    for (int j = 1; j <= windowSize && j <= numberOfFrames; j++) {
        cout << "Sending Frame " << j << endl;
    }

    // Main loop to send frames
    for (int i = 1; i <= numberOfFrames; i = lastSuccessfulFrame + 1) {
        for (int j = i; j < i + windowSize && j <= numberOfFrames; j++) {
            int random = rand() % 100;
            if (random < wireEfficiency) {
                cout << "Frame " << j << " sent successfully" << endl;
                lastSuccessfulFrame = j;
                if (j + windowSize <= numberOfFrames) {
                    cout << "Sending Frame " << j + windowSize << endl;
                }
            } else {
                cout << "Frame " << j << " not sent" << endl;
                // Resend frames within the window size
                for (int k = j; k < j + windowSize && k <= numberOfFrames; k++) {
                    cout << "Resending Frame " << k << endl;
                }
                break;
            }
        }
    }
}