#include "go_back_n.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>

using namespace std;

int window_size;  // Defined globally
bool timeout = true;
bool network_layer_enabled = true;

// Increment the sequence number with wraparound based on the window size
void increment(int& k, int MAX_SEQ) {
    k = (k + 1) % (MAX_SEQ + 1);  // Wrap around based on MAX_SEQ
}

// Send data to physical layer
void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[], bool is_nak = false) {
    Frame s;
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);  // Piggyback ACK
    s.is_nak = is_nak;
    memcpy(s.data, buffer[frame_nr].data, MAX_PKT);  // Copy packet data to frame
    to_physical_layer(s);
    start_timer(frame_nr);
}

// Simulate sending a frame to physical layer
void to_physical_layer(Frame& frame) {
    if (frame.is_nak) {
        cout << "Sending NAK for frame [Expected: " << frame.ack << "] to physical layer\n";
    } else {
        cout << "Sending frame [Seq: " << frame.seq << ", Ack: " << frame.ack << "] to physical layer\n";
    }
}

// Simulate receiving a frame from physical layer with optional error/loss
bool from_physical_layer(Frame& frame, bool simulate_loss = false, bool simulate_error = false) {
    static int call_count = 0;
    call_count++;

    // Simulate frame loss (every second frame)
    if (simulate_loss && call_count % 2 == 0) {
        return false;  // Simulate frame loss
    }

    // Simulate checksum error (e.g., wrong sequence number)
    if (simulate_error && call_count % 3 == 0) {
        frame.seq = (frame_expected + 1) % (MAX_SEQ + 1);  // Wrong seq number
    } else {
        frame.seq = rand() % (MAX_SEQ + 1);  // Random seq number
    }

    frame.ack = rand() % (MAX_SEQ + 1);  // Random ack number
    memcpy(frame.data, "ReceivedData", 12);  // Example payload
    return true;
}

// Simulate fetching a packet from network layer
void from_network_layer(packet& p) {
    for (int i = 0; i < MAX_PKT; i++) {
        p.data[i] = 'A' + (rand() % 26);  // Random data
    }
}

// Simulate sending data to network layer
void to_network_layer(char data[]) {
    cout << "Data passed to network layer: " << data << endl;
}

// Simulate starting a timer for a given frame
void start_timer(int seq) {
    cout << "Starting timer for frame " << seq << endl;
    timeout = true;
}

// Simulate stopping a timer for a given frame
void stop_timer(int seq) {
    cout << "Stopping timer for frame " << seq << endl;
    timeout = false;
}

// Calculate the window size based on bandwidth, delay, and frame size
int calculate_window_size(int bandwidth, double propagation_delay, int frame_size) {
    double BDP_in_bits = bandwidth * propagation_delay;
    int BDP_in_frames = static_cast<int>(ceil(BDP_in_bits / frame_size));  // Bandwidth-Delay Product
    window_size = 1 + 2 * BDP_in_frames;  // Calculate window size
    return window_size;
}

// Enable the network layer
void enable_network_layer() {
    network_layer_enabled = true;
    cout << "Network layer enabled.\n";
}

// Disable the network layer
void disable_network_layer() {
    network_layer_enabled = false;
    cout << "Network layer disabled.\n";
}

// Wait for an event (randomly generate events)
void wait_for_event(event_type* event) {
    int random_event = rand() % 4;  // Randomly pick an event
    *event = static_cast<event_type>(random_event);
}

// Check if a sequence number is between two others
bool between(seq_nr a, seq_nr b, seq_nr c) {
    return ((a <= b && b < c) || (a <= b || b < c));
}

// Go-Back-N sender logic
void go_back_n_sender(int MAX_SEQ) {
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    packet buffer[MAX_SEQ + 1];  // Buffer to store the frames
    seq_nr nbuffered = 0;
    seq_nr i;
    event_type event;

    enable_network_layer();

    while (true) {
        wait_for_event(&event);  // Wait for an event

        switch (event) {
            case network_layer_ready:
                if (nbuffered < MAX_SEQ) {
                    from_network_layer(buffer[next_frame_to_send]);  // Fetch packet from network
                    send_data(next_frame_to_send, frame_expected, buffer);  // Send data
                    nbuffered++;
                    increment(next_frame_to_send, MAX_SEQ);  // Increment frame number
                }
                break;

            case frame_arrival:
                Frame received_frame;
                if (from_physical_layer(received_frame)) {
                    if (received_frame.seq == frame_expected) {
                        cout << "Frame " << received_frame.seq << " received correctly" << endl;
                        to_network_layer(received_frame.data);  // Pass data to network layer
                        frame_expected = (frame_expected + 1) % MAX_SEQ;  // Update expected frame

                        stop_timer(ack_expected);

                        while (between(ack_expected, received_frame.ack, next_frame_to_send)) {
                            nbuffered--;
                            stop_timer(ack_expected);
                            increment(ack_expected, MAX_SEQ);
                            if (nbuffered < MAX_SEQ) enable_network_layer();
                        }
                    } else {
                        cout << "Frame " << received_frame.seq << " not expected. Sending NAK...\n";
                        send_data(received_frame.seq, frame_expected, buffer, true);  // Send NAK
                    }
                }
                break;

            case timeout:
                for (i = ack_expected; i != next_frame_to_send; increment(i, MAX_SEQ)) {
                    send_data(i, frame_expected, buffer);  // Retransmit frames due to timeout
                }
                break;

            case cksum_err:
                break;

            default:
                break;
        }

        if (nbuffered < MAX_SEQ) enable_network_layer();
        else disable_network_layer();
    }
}

// Reset the test case state
void reset_test_case() {
    timeout = true;
    network_layer_enabled = true;
    // Reset other state as needed
}

// Run test case 1 (Normal Operation)
void run_test_case_1() {
    reset_test_case();
    cout << "\nTest Case 1: Normal Operation with Sequence of Frames" << endl;
    cout << "----------------------------------------------------" << endl;
    go_back_n_sender(MAX_SEQ);
}

// Other test cases (run_test_case_2 to run_test_case_10) can be written in similar manner as run_test_case_1
