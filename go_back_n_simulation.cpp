#include "go_back_n.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>


using namespace std;

int window_size;

// Increment the sequence number circularly based on the window size (w)
void increment(int& k, int MAX_SEQ) {
    k = (k + 1) % MAX_SEQ;
     /* alternatively we can use:
     if (k < MAX_SEQ)
        k++;
    else
        k = 0;
     */
}

// Function to send data with piggybacked acknowledgment
void send_data(seq_nr frame_nr, seq_nr frame_expected, packet buffer[]) {
    Frame s;
    s.seq = frame_nr;                               // Insert sequence number into frame
    s.ack = (frame_expected + MAX_SEQ) % (MAX_SEQ + 1); // Piggyback ack
    memcpy(s.data, buffer[frame_nr], MAX_PKT);      // Insert packet into frame
    to_physical_layer(s);                           // Send the frame
    start_timer(frame_nr);                          // Start timer for frame
}



// Simulate sending a frame to the physical layer
void to_physical_layer(Frame& frame) {
    cout << "Sending frame [Seq: " << frame.seq << ", Ack: " << frame.ack << "] to physical layer\n";
}

// Simulate receiving a frame from the physical layer
bool from_physical_layer(Frame& frame) {
    // In a real implementation, you'd fetch the frame from the channel.
    // For simulation purposes: We will simulate a valid frame arrival (for demonstration)
    frame.seq = rand() % (MAX_SEQ + 1);     // Random sequence number
    frame.ack = rand() % (MAX_SEQ + 1);     // Random acknowledgment number
    memcpy(frame.data, "ReceivedData", 12); // Simulate data
    return true;                            // Frame received
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
bool timeout = true;
// Simulate starting a timer for the frame
void start_timer(int seq) {
    cout << "Starting timer for frame " << seq << endl;
    this_thread::sleep_for(chrono::seconds(1));
    if(timeout)
        cout << "Timeout for frame: " << seq << endl;
}

// Simulate stopping a timer for the frame
void stop_timer(int seq) {
    cout << "Stopping timer for frame " << seq << endl;
    timeout = false;
    /*
    When a frame is lost during transmission, if its ack is not received within a certain time, re-transmission starting
    from this packet and all subsequent ones is triggered even if they have been delivered but once the ack is received 
    the sender stops the timer and no timeout occurs.
    */
}

// Calculate the optimal window size based on bandwidth and delay
//Must be called before starting the protocol to set the protocol specifications such bandwidth,frame_size and window_size
void calculate_window_size(int bandwidth, double propagation_delay, int frame_size) {
    // Bandwidth-Delay Product (BDP) in bits
    double BDP_in_bits = bandwidth * propagation_delay;

    // Convert BDP to number of frames
    int BDP_in_frames = static_cast<int>(ceil(BDP_in_bits / frame_size));

    window_size = 1 + 2 * BDP; //window_size is defined in go_back_n_simulation.h, setting MAX_SEQ with window_size
}

// Go-Back-N sender logic
void go_back_n_sender(int MAX_SEQ) {
    seq_nr next_frame_to_send = 0;     // Next frame to send
    seq_nr ack_expected = 0;            // Oldest unacknowledged frame
    seq_nr frame_expected = 0;          // Next frame expected on inbound stream
    packet buffer[MAX_SEQ + 1];         // Buffer to hold frames till receiving their ack
    seq_nr nbuffered = 0;               // Number of current buffer frames
    seq_nr i;                           // Index used to iterate through buffer
    event_type event;                   // Event type placeholder


    enable_network_layer();             // Allow network layer to be ready

    // Simulate the sender operation
    while (true) {

        wait_for_event(&event);         // Wait for an event

        switch (event) {

        case network_layer_ready: {
            if (nbuffered < MAX_SEQ) {
                // Fetch a new packet from the network layer
                from_network_layer(buffer);
                send_data(next_frame_to_send, frame_expected, buffer);  // Send data frame

                // Increment the number of buffered frames and move to the next frame to send
                nbuffered++;
                increment(next_frame_to_send, MAX_SEQ);
                break;

            }
            // Simulate a timeout event (retransmit all frames)
            else if (nbuffered == MAX_SEQ) {
                    cout << "Window full, retransmitting frames starting from " << ack_expected << endl;
                    for (int i = ack_expected; i != next_frame_to_send; increment(i, MAX_SEQ)) {
                        to_physical_layer(buffer[i]);
                        start_timer(i);
                    }
                    break;
                }
        }

        case frame_arrival: {
            // Simulate receiving a frame (acknowledgment or data)
            Frame received_frame;
            if (from_physical_layer(received_frame)) {
                // Check if the received frame is the one expected
                if (received_frame.seq == frame_expected) {
                    cout << "Frame " << received_frame.seq << " received correctly" << endl;

                    //pass the frame to network layer
                    to_network_layer(r.data); // Pass packet to network layer 
                  
                    // Deliver the packet to the network layer (acknowledge)
                    frame_expected = (frame_expected + 1) % MAX_SEQ;

                    // Stop the timer for this frame
                    stop_timer(ack_expected);



                    // Slide the window (acknowledge previous frames)
                    /*
                    This loop acknowledges all frames up to the expected one. Since Go-Back-N uses cumulative acknowledgments,
                    once a frame is received correctly, all previous unacknowledged frames are implicitly acknowledged.
                    */
                    while (ack_expected != frame_expected) {
                        // Handle acknowledgment piggybacked on frames
                        while (between(ack_expected, r.ack, next_frame_to_send)) {
                            nbuffered--; // Decrease the number of buffered frames
                            stop_timer(ack_expected); // Stop timer for this frame
                            increment(ack_expected, MAX_SEQ);
                        }
                        
                    }
                    break;
                }
                /***************************************************************************/
                //handle case when received_frame.seq != frame_expected
                else if(received_frame.seq != frame_expected) {

                }
            }
           
        }
        case timeout: {
            // Retransmit all unacknowledged frames
            seq_nr resend_frame = ack_expected;
            for (i = 1; i <= nbuffered; i++) {
                send_data(resend_frame, frame_expected, buffer); // Resend frame
                increment(resend_frame, MAX_SEQ); // Prepare next frame for retransmission
            }
            break;
        }

        case cksum_err:
            break;  // Ignore bad frames

        default:
            break;
        }


        // Enable or disable network layer depending on window size
        if (nbuffered < MAX_SEQ) enable_network_layer();
        else disable_network_layer();
    }
}



