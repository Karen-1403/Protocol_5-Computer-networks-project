#include "go_back_n.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>

using namespace std;

int window_size;
bool timeout = true;
bool network_layer_enabled = true;

void increment(int& k, int MAX_SEQ) {
    k = (k + 1) % MAX_SEQ;
}

void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[], bool is_nak = false) {
    Frame s;
    s.seq = frame_nr;
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1);
    s.is_nak = is_nak;
    memcpy(s.data, buffer[frame_nr].data, MAX_PKT);
    to_physical_layer(s);
    start_timer(frame_nr);
}

void to_physical_layer(Frame& frame) {
    if (frame.is_nak) {
        cout << "Sending NAK for frame [Expected: " << frame.ack << "] to physical layer\n";
    }
    else {
        cout << "Sending frame [Seq: " << frame.seq << ", Ack: " << frame.ack << "] to physical layer\n";
    }
}

bool from_physical_layer(Frame& frame) {
    frame.seq = rand() % (MAX_SEQ + 1);
    frame.ack = rand() % (MAX_SEQ + 1);
    memcpy(frame.data, "ReceivedData", 12);
    return true;
}

void from_network_layer(packet& p) {
    for (int i = 0; i < MAX_PKT; i++) {
        p.data[i] = 'A' + (rand() % 26);
    }
}

void to_network_layer(char data[]) {
    cout << "Data passed to network layer: " << data << endl;
}

void start_timer(int seq) {
    cout << "Starting timer for frame " << seq << endl;
    this_thread::sleep_for(chrono::seconds(1));
    if (timeout)
        cout << "Timeout for frame: " << seq << endl;
}

void stop_timer(int seq) {
    cout << "Stopping timer for frame " << seq << endl;
    timeout = false;
}

int calculate_window_size(int bandwidth, double propagation_delay, int frame_size) {
    double BDP_in_bits = bandwidth * propagation_delay;
    int BDP_in_frames = static_cast<int>(ceil(BDP_in_bits / frame_size));
    return window_size = 1 + 2 * BDP_in_frames;
}

void enable_network_layer() {
    network_layer_enabled = true;
    cout << "Network layer enabled.\n";
}

void disable_network_layer() {
    network_layer_enabled = false;
    cout << "Network layer disabled.\n";
}

void wait_for_event(event_type* event) {
    int random_event = rand() % 4;
    *event = static_cast<event_type>(random_event);
}

void go_back_n_sender(int MAX_SEQ) {
    seq_nr next_frame_to_send = 0;
    seq_nr ack_expected = 0;
    seq_nr frame_expected = 0;
    packet buffer[MAX_SEQ + 1];
    seq_nr nbuffered = 0;
    seq_nr i;
    event_type event;

    enable_network_layer();

    while (true) {
        wait_for_event(&event);

        switch (event) {
        case network_layer_ready:
            if (nbuffered < MAX_SEQ) {
                from_network_layer(buffer[next_frame_to_send]);
                send_data(next_frame_to_send, frame_expected, buffer);
                nbuffered++;
                increment(next_frame_to_send, MAX_SEQ);
            }
            break;

        case frame_arrival:
            Frame received_frame;
            if (from_physical_layer(received_frame)) {
                if (received_frame.seq == frame_expected) {
                    cout << "Frame " << received_frame.seq << " received correctly" << endl;
                    to_network_layer(received_frame.data);
                    frame_expected = (frame_expected + 1) % MAX_SEQ;
                    stop_timer(ack_expected);

                    while (between(ack_expected, received_frame.ack, next_frame_to_send)) {
                        nbuffered--;
                        stop_timer(ack_expected);
                        increment(ack_expected, MAX_SEQ);
                        if (nbuffered < MAX_SEQ) enable_network_layer();
                    }
                }
                else {
                    cout << "Frame " << received_frame.seq << " not expected. Sending NAK...\n";
                    send_data(received_frame.seq, frame_expected, buffer, true);
                }
            }
            break;

        case timeout:
            for (i = ack_expected; i != next_frame_to_send; increment(i, MAX_SEQ)) {
                send_data(i, frame_expected, buffer);
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

void run_test_case_1() {
    cout << "\nTest Case 1: Normal Operation with Sequence of Frames" << endl;
    cout << "----------------------------------------------------" << endl;
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_2() {
    cout << "\nTest Case 2: Simulate Timeout" << endl;
    cout << "--------------------------------" << endl;
    timeout = true;
    start_timer(0); // Manually trigger a timeout
}

void run_test_case_3() {
    cout << "\nTest Case 3: Handle Frame Loss" << endl;
    cout << "----------------------------------" << endl;
    bool from_physical_layer(Frame & frame) {
        static int call_count = 0;
        call_count++;
        if (call_count == 3) {
            return false; // Simulate frame loss on the third call
        }
        frame.seq = rand() % (MAX_SEQ + 1);
        frame.ack = rand() % (MAX_SEQ + 1);
        memcpy(frame.data, "ReceivedData", 12);
        return true;
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_4() {
    cout << "\nTest Case 4: Check NAK Handling" << endl;
    cout << "------------------------------------" << endl;
    bool from_physical_layer(Frame & frame) {
        static int call_count = 0;
        call_count++;
        if (call_count % 3 == 0) {
            frame.seq = (frame_expected + 1) % (MAX_SEQ + 1); // Simulate wrong seq number
        }
        else {
            frame.seq = frame_expected;
        }
        frame.ack = rand() % (MAX_SEQ + 1);
        memcpy(frame.data, "ReceivedData", 12);
        return true;
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_5() {
    cout << "\nTest Case 5: Handle Multiple Retransmissions" << endl;
    cout << "---------------------------------------------" << endl;
    bool from_physical_layer(Frame & frame) {
        static int call_count = 0;
        call_count++;
        if (call_count % 2 == 0) {
            return false; // Simulate loss of every second frame
        }
        frame.seq = rand() % (MAX_SEQ + 1);
        frame.ack = rand() % (MAX_SEQ + 1);
        memcpy(frame.data, "ReceivedData", 12);
        return true;
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_6() {
    cout << "\nTest Case 6: Continuous Normal Operation" << endl;
    cout << "-----------------------------------------" << endl;
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_7() {
    cout << "\nTest Case 7: Simulate Corrupted Frame" << endl;
    cout << "--------------------------------------" << endl;
    bool from_physical_layer(Frame & frame) {
        frame.seq = rand() % (MAX_SEQ + 1);
        frame.ack = rand() % (MAX_SEQ + 1);
        memcpy(frame.data, "ReceivedData", 12);
        return rand() % 5 != 0; // Simulate corruption with 20% chance
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_8() {
    cout << "\nTest Case 8: Immediate Acknowledgment" << endl;
    cout << "---------------------------------------" << endl;
    bool from_physical_layer(Frame & frame) {
        frame.seq = frame_expected;
        frame.ack = (frame_expected + 1) % (MAX_SEQ + 1);
        memcpy(frame.data, "ReceivedData", 12);
        return true;
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_9() {
    cout << "\nTest Case 9: Large Window Size" << endl;
    cout << "--------------------------------" << endl;
    int calculate_window_size(int bandwidth, double propagation_delay, int frame_size) {
        return window_size = 10; // Set a large window size for testing
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_case_10() {
    cout << "\nTest Case 10: Small Window Size" << endl;
    cout << "---------------------------------" << endl;
    int calculate_window_size(int bandwidth, double propagation_delay, int frame_size) {
        return window_size = 2; // Set a small window size for testing
    }
    go_back_n_sender(MAX_SEQ);
}

void run_test_cases() {
    run_test_case_1();
    run_test_case_2();
    run_test_case_3();
    run_test_case_4();
    run_test_case_5();
    run_test_case_6();
    run_test_case_7();
    run_test_case_8();
    run_test_case_9();
    run_test_case_10();
}
int main() {
    run_test_cases();
    return 0;
}
