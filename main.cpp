#include "protocol_5.cpp"
#include "go_back_n_protocol.h"
#include <iostream>

using namespace std;

int main() {
    /*
    //Go_Back_N_Protocol testing (Not Working)
    cout << "Running test cases for Go-Back-N ARQ protocol implementation...\n";

    test_basic_data_transmission();
    test_frame_arrival_correct_sequence();
    test_frame_arrival_incorrect_sequence();
    test_timeout_handling();
    test_checksum_error_handling();

    cout << "Test cases completed.\n";
    return 0;
    */

    /*---------------------------------------------------------------------------*/
    //protocol5 testing (Working)
    
    int windowSize, numberOfFrames, wireEfficiency;

    // Get user input with basic validation
    cout << "Enter window size: ";
    while (!(cin >> windowSize) || windowSize <= 0) {
        cout << "Please enter a valid positive integer for window size: ";
        cin.clear();
        cin.ignore(10000, '\n'); // Clear the input buffer
    }

    cout << "Enter number of frames to be sent: ";
    while (!(cin >> numberOfFrames) || numberOfFrames <= 0) {
        cout << "Please enter a valid positive integer for number of frames: ";
        cin.clear();
        cin.ignore(10000, '\n'); // Clear the input buffer
    }

    cout << "Enter wire efficiency (percentage that frames will be sent successfully): ";
    while (!(cin >> wireEfficiency) || wireEfficiency < 0 || wireEfficiency > 100) {
        cout << "Please enter a valid percentage (0-100) for wire efficiency: ";
        cin.clear();
        cin.ignore(10000, '\n'); // Clear the input buffer
    }

    // Call the protocol function
    protocol5(windowSize, numberOfFrames, wireEfficiency);

    return 0;

}
