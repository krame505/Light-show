/*
 Copyright (C) 2014 smellyworkshop@github.com

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * rf24_monitor
 *
 * This displays and prints packates in hex and decimal format.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration
#define CE_PIN 15    // Nano A1 pin, functions same as D15
#define CSE_PIN 16   // Nano A1 pin, functions same as D16

// Set up nRF24L01 radio on SPI bus plus CE and CSE pins
RF24 radio(15,16); //light show radio config

// Single radio pipe address for the 2 nodes to communicate.
//const uint64_t pipe = 0xE8E8F0F0E1LL;
const uint64_t pipe = 0xF0F0F0F0E1LL;

// Payload
#define PACKET_LENGTH 8
uint8_t packet[PACKET_LENGTH];

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/rf24_monitor/\n\r");

  // Setup and configure rf radio
  radio.begin();
  radio.openReadingPipe(1,pipe);  // Open pipes to other nodes for communication
  radio.startListening();  // Start listening
  radio.printDetails();  // Dump the configuration of the rf unit for debugging
}

void loop(void)
{
  // if there is data ready
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done)
    {
      unsigned long rx_time_ms = millis();

      // Fetch the payload, and see if this was the last one.
      done = radio.read(packet, PACKET_LENGTH);

      // Print the time
      printf("%3d",rx_time_ms/1000);
      printf(".%03d [",rx_time_ms%1000);

     // Print the packet in hex, then decimal
      for (int i = 0; i < PACKET_LENGTH; i++)
      {
        printf("%02x ",packet[i]);
      }
      printf("] ");
      for (int i = 0; i < PACKET_LENGTH; i++)
      {
        printf("%3d ",packet[i]);
      }
      printf("\n\r");
    }
  }
}
// vim:ai:cin:sts=2 sw=2 ft=cpp
