#ifndef GO_BACK_N_H
#define GO_BACK_N_H

#include <iostream>
#include <cmath> // For ceil
#include <cstring> // For memcpy alternative
#include <vector> // For dynamic array

#define MAX_PKT 1024 // Packet size in bytes

typedef int seq_nr;
typedef char packet[MAX_PKT];

// Frame structure
struct Frame {
    seq_nr seq; // Sequence number
    seq_nr ack; // Acknowledgment number
    char data[MAX_PKT]; // Payload (data)
};

enum event_type {
    network_layer_ready, // Network layer has a new packet to send
    frame_arrival, // A frame has arrived
    timeout, // A timeout occurred
    cksum_err, // Checksum error (frame is corrupt)
    other_event // Any other event
};

void increment(int& k, int window_size); // Increment sequence number circularly with window size
void to_physical_layer(Frame& frame); // Simulate sending a frame to physical layer for transmission
bool from_physical_layer(Frame& frame); // Simulate receiving a frame from physical layer
void from_network_layer(char data[]); // Simulate fetching a packet from network layer
void to_network_layer(char data[]);
void start_timer(int seq); // Simulate starting a timer
void stop_timer(int seq); // Simulate stopping a timer
int calculate_window_size(int bandwidth, double delay, int frame_size); // Calculate optimal window size
void go_back_n_sender(int w); // Go-Back-N sender logic with window size
void send_data(seq_nr frame_nr, seq_nr frame_expected, std::vector<packet>& buffer); // Send data with piggybacking ack
void enable_network_layer(); // Enables the network layer
void disable_network_layer(); // Disables the network layer

//testing

void test_basic_data_transmission();
void test_frame_arrival_correct_sequence();
void test_frame_arrival_incorrect_sequence();
void test_timeout_handling();
void test_checksum_error_handling();

#endif // GO_BACK_N_H