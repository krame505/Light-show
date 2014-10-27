/*
 Copyright (C) 2014 by smellyworkshop@github.com
 Modified by smellyworkshop 2014-08-17
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#include <cstdlib>
#include <iostream>
#include <RF24.h>

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus 
// spi device, speed and CSN,only CSN is NEEDED in RPI
RF24 radio("/dev/spidev0.0",8000000 , 25);  

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
//match led_remote_ls address
//const uint64_t pipes[2] = { 0xE8E8F0F0E1LL, 0xF0F0F0F0D2LL };


#define PACKET_LENGTH 8
//uint8_t packet [PACKET_LENGTH] = {1,2,3,4,5,6,7,8};
// command LC1 to turn on every other relay 
uint8_t packet [PACKET_LENGTH] = {0x20,2,0x55,0,0,0,0,0}; //bytes swapped
//uint8_t packet [PACKET_LENGTH] = {0,0,0,0,0,0x55,2,0x10};

void setup(void)
{
  printf("\n\rnRF24l01+ remotecmd\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
//  radio.setPayloadSize(PACKET_LENGTH);
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);

  //
  // Open pipes to other nodes for communication
  //

  // Open pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

}

void loop(void)
{
    // First, stop listening so we can talk.
    radio.stopListening();

    bool ok = radio.write( &packet, PACKET_LENGTH);
    
    if (ok)
      printf("sent...%d\n\r",packet[2]);
    else
      printf("failed.\n\r");

	sleep(1);
	packet[2]++;
//	packet[5]++;
}

int main(int argc, char** argv)
{
		printf("0)%s  1)%s  2)%s\n\r", argv[0], argv[1], argv[2]);
        setup();
        while(1)
                loop();
        return 0;
}
