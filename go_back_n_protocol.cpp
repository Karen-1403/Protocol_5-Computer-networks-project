#include "go_back_n_protocol.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>

using namespace std;

int window_size = 0;

// Increment the sequence number circularly based on the window size (w)
void increment(int& k, int window_size) {
    int MAX_SEQ = window_size - 1;
    if (MAX_SEQ > 0) {
        k = (k + 1) % (MAX_SEQ + 1);
    } else {
        cout << "Error in increment: MAX_SEQ is zero. Ensure window_size is properly initialized." << endl;
    }
}

// Function to send data with piggybacked acknowledgment
void send_data(seq_nr frame_nr, seq_nr frame_expected, vector<packet>& buffer) {
    Frame s;
    int MAX_SEQ = window_size - 1;
    if (MAX_SEQ <= 0) {
        cout << "Error in send_data: MAX_SEQ is zero. Ensure window_size is properly initialized." << endl;
        return;
    }
    s.seq = frame_nr; // Insert sequence number into frame
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1); // Piggyback ack
    memcpy(s.data, buffer[frame_nr], MAX_PKT); // Insert packet into frame
    to_physical_layer(s); // Send the frame
    start_timer(frame_nr); // Start timer for frame
}

// Simulate sending a frame to the physical layer
void to_physical_layer(Frame& frame) {
    cout << "Sending frame [Seq: " << frame.seq << ", Ack: " << frame.ack << "] to physical layer\n";
}

// Simulate receiving a frame from the physical layer
bool from_physical_layer(Frame& frame) {
    int MAX_SEQ = window_size - 1;
    if (MAX_SEQ <= 0) {
        cout << "Error in from_physical_layer: MAX_SEQ is zero. Ensure window_size is properly initialized." << endl;
        return false;
    }
    // In a real implementation, you'd fetch the frame from the channel.
    // For simulation purposes: We will simulate a valid frame arrival (for demonstration)
    frame.seq = rand() % (MAX_SEQ + 1); // Random sequence number
    frame.ack = rand() % (MAX_SEQ + 1); // Random acknowledgment number
    memcpy(frame.data, "ReceivedData", 12); // Simulate data
    return true; // Frame received
}

// Simulate fetching a packet from the network layer to send as a frame
void from_network_layer(char data[]) {
    // Normally, this function fetches a packet from the network layer.
    // For simulation purposes, we're just generating random data.
    // Generate random dummy data
    for (int i = 0; i < MAX_PKT; i++) {
        data[i] = 'A' + (rand() % 26); // Random letters
    }
}

void to_network_layer(char data[]) {
    // Simulating passing the data to the network layer
    // In a real scenario, you could forward the data to an upper layer or process it further
    cout << "Data received at network layer: " << data << endl;
}

// Simulate starting a timer
void start_timer(int seq) {
    cout << "Starting timer for sequence number: " << seq << endl;
}

// Simulate stopping a timer
void stop_timer(int seq) {
    cout << "Stopping timer for sequence number: " << seq << endl;
}

// Calculate optimal window size
int calculate_window_size(int bandwidth, double delay, int frame_size) {
    return ceil((bandwidth * delay) / frame_size);
}

// Go-Back-N sender logic with window size
void go_back_n_sender(int w) {
    window_size = w;
    if (window_size <= 0) {
        cout << "Error: window_size must be greater than zero." << endl;
        return;
    }
    cout << "Initialized window_size: " << window_size << endl;
    vector<packet> buffer(window_size);
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    int nbuffered = 0;

    enable_network_layer();

    while (true) {
        event_type event;
        // Simulate event handling (for demonstration purposes)
        event = static_cast<event_type>(rand() % 5);

        switch (event) {
            case network_layer_ready:
                from_network_layer(buffer[next_frame_to_send]);
                nbuffered++;
                send_data(next_frame_to_send, frame_expected, buffer);
                increment(next_frame_to_send, window_size);
                break;

            case frame_arrival:
                Frame r;
                if (from_physical_layer(r)) {
                    if (r.seq == frame_expected) {
                        to_network_layer(r.data);
                        increment(frame_expected, window_size);
                    }
                    while ((ack_expected <= r.ack) && (r.ack < next_frame_to_send)) {
                        nbuffered--;
                        stop_timer(ack_expected);
                        increment(ack_expected, window_size);
                    }
                }
                break;

            case cksum_err:
                cout << "Checksum error, frame is corrupt.\n";
                break;

            case timeout:
                next_frame_to_send = ack_expected;
                for (int i = 0; i < nbuffered; i++) {
                    send_data(next_frame_to_send, frame_expected, buffer);
                    increment(next_frame_to_send, window_size);
                }
                break;

            case other_event:
                cout << "Handling other events.\n";
                break;
        }

        if (nbuffered < window_size) {
            enable_network_layer();
        } else {
            disable_network_layer();
        }
    }
}

void enable_network_layer() {
    cout << "Network layer enabled.\n";
}

void disable_network_layer() {
    cout << "Network layer disabled.\n";
}

/*---------------------------------------------------------------------------------------------------------*/
/*Test bench*/

void test_basic_data_transmission() {
    int bandwidth = 1000; // in bits per second
    double delay = 0.1; // in seconds
    int frame_size = MAX_PKT * 8; // in bits

    int window_size = calculate_window_size(bandwidth, delay, frame_size);
    cout << "Window size: " << window_size << endl;

    go_back_n_sender(window_size);
}

void test_frame_arrival_correct_sequence() {
    int window_size = 4; // Example window size
    vector<packet> buffer(window_size);
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    int nbuffered = 0;

    enable_network_layer();

    // Simulate network layer ready event
    from_network_layer(buffer[next_frame_to_send]);
    nbuffered++;
    send_data(next_frame_to_send, frame_expected, buffer);
    increment(next_frame_to_send, window_size);

    // Simulate frame arrival event
    Frame r;
    r.seq = frame_expected;
    r.ack = ack_expected;
    memcpy(r.data, "TestData", 8);

    if (from_physical_layer(r)) {
        if (r.seq == frame_expected) {
            to_network_layer(r.data);
            increment(frame_expected, window_size);
        }
        while ((ack_expected <= r.ack) && (r.ack < next_frame_to_send)) {
            nbuffered--;
            stop_timer(ack_expected);
            increment(ack_expected, window_size);
        }
    }
}


void test_frame_arrival_incorrect_sequence() {
    int window_size = 4; // Example window size
    vector<packet> buffer(window_size);
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    int nbuffered = 0;

    enable_network_layer();

    // Simulate network layer ready event
    from_network_layer(buffer[next_frame_to_send]);
    nbuffered++;
    send_data(next_frame_to_send, frame_expected, buffer);
    increment(next_frame_to_send, window_size);

    // Simulate frame arrival event with incorrect sequence
    Frame r;
    r.seq = (frame_expected + 1) % window_size; // Incorrect sequence number
    r.ack = ack_expected;
    memcpy(r.data, "TestData", 8);

    if (from_physical_layer(r)) {
        if (r.seq == frame_expected) {
            to_network_layer(r.data);
            increment(frame_expected, window_size);
        } else {
            cout << "Out-of-order frame received. Expected: " << frame_expected << ", Received: " << r.seq << endl;
        }
        while ((ack_expected <= r.ack) && (r.ack < next_frame_to_send)) {
            nbuffered--;
            stop_timer(ack_expected);
            increment(ack_expected, window_size);
        }
    }
}

void test_timeout_handling() {
    int window_size = 4; // Example window size
    vector<packet> buffer(window_size);
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    int nbuffered = 0;

    enable_network_layer();

    // Simulate network layer ready event
    from_network_layer(buffer[next_frame_to_send]);
    nbuffered++;
    send_data(next_frame_to_send, frame_expected, buffer);
    increment(next_frame_to_send, window_size);

    // Simulate timeout event
    cout << "Simulating timeout event.\n";
    next_frame_to_send = ack_expected;
    for (int i = 0; i < nbuffered; i++) {
        send_data(next_frame_to_send, frame_expected, buffer);
        increment(next_frame_to_send, window_size);
    }
}

void test_checksum_error_handling() {
    int window_size = 4; // Example window size
    vector<packet> buffer(window_size);
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    int nbuffered = 0;

    enable_network_layer();

    // Simulate network layer ready event
    from_network_layer(buffer[next_frame_to_send]);
    nbuffered++;
    send_data(next_frame_to_send, frame_expected, buffer);
    increment(next_frame_to_send, window_size);

    // Simulate checksum error event
    cout << "Simulating checksum error event.\n";
    Frame r;
    r.seq = frame_expected;
    r.ack = ack_expected;
    memcpy(r.data, "CorruptedData", 13);

    if (!from_physical_layer(r)) {
        cout << "Checksum error, frame is corrupt.\n";
    }
}

int main() {
    cout << "Running test cases for Go-Back-N ARQ protocol implementation...\n";

    test_basic_data_transmission();
    test_frame_arrival_correct_sequence();
    test_frame_arrival_incorrect_sequence();
    test_timeout_handling();
    test_checksum_error_handling();

    cout << "Test cases completed.\n";
    return 0;
}