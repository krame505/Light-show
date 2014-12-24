/*
 Copyright (C) 2014 by smellyworkshop@github.com
 Modified by smellyworkshop 2014-08-17
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

#define VERBOSE
#define DEBUG
#include <cstdlib>
#include <iostream>
#include <RF24.h>

// Hardware configuration

// Set up nRF24L01 radio on SPI bus 
// spi device, speed and CSN,only CSN is NEEDED in RPI
RF24 radio("/dev/spidev0.0",8000000 , 25);  

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };  	// GettingStarted
//const uint64_t pipes[2] = { 0xE8E8F0F0E1LL, 0xF0F0F0F0D2LL }; // led_remote_ls

#define PACKET_LENGTH 8
uint8_t packet [PACKET_LENGTH] = {0x20,2,0x55,0,0,0,0,0};
//bool autobroadcast = true;
bool autobroadcast = false;
float delay_s = 1;
unsigned long total_time_us = 0;
unsigned long delay_ms = 1000;
unsigned long delay_us = 1000000;

void setup(void)
{
  printf("\n\rnRF24l01+ remotecmd\n\r");

  // Setup and configure rf radio
  radio.begin();
  radio.setRetries(15,15);	// optionally, increase the delay between retries & # of retries
  // optionally, reduce the payload size.  seems to improve reliability
//  radio.setPayloadSize(PACKET_LENGTH);
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_MAX);

  // Open pipes to other nodes for communication
  // Open pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
  radio.openWritingPipe(pipes[0]);
//  radio.openReadingPipe(1,pipes[1]);

  // Initalize delta timestamp
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  total_time_us = current_time.tv_sec * 1000000 + current_time.tv_usec;
}
char str[256];
uint32_t line = 0;

void loop(void)
{
	line++;
    // First, stop listening so we can talk.
//    radio.stopListening();
	int num_scanned = scanf("%f%*[, ]%hhx%*[, ]%hhx%*[, ]%hhx%*[, ]%hhx%*[, ]%hhx%*[, ]%hhx%*[, ]%hhx%*[, ]%hhx%*[, ]",
		&delay_s,&packet[0],&packet[1],&packet[2],&packet[3],
		&packet[4],&packet[5],&packet[6],&packet[7]);
//printf("%f,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,  ",
#ifdef VERBOSE  
printf("%f [%02x %02x %02x %02x %02x %02x %02x %02x]  ",
		delay_s,packet[0],packet[1],packet[2],packet[3],
		packet[4],packet[5],packet[6],packet[7]);
	printf("%s",fgets(str,sizeof(str),stdin));
#else
  fgets(str,sizeof(str),stdin);
#endif //VERBOSE
	
	if (num_scanned == -1) {
		printf("\n\rfinished\n\r");
		exit (0);
	} else if (num_scanned < 9) {
		printf("\n\rline %d -->expected input format: delay (seconds), message ID, data1,...,data7[, optional comment]\n\r",line);
		printf("aborting with error\n\r");
		exit (-1);
	}
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  total_time_us += delay_s * 1000000;
  if (total_time_us > (unsigned long)(current_time.tv_sec * 1000000 + current_time.tv_usec))
    delay_us = total_time_us - (current_time.tv_sec * 1000000 + current_time.tv_usec);
  else delay_us = 0;
	bool ok = radio.write( &packet, PACKET_LENGTH);
#ifdef DEBUG
    if (ok)
      printf("line %d -->tx delay: %f s\n\r", line, delay_us/1000000.0);
#endif //DEBUG
#ifdef VERBOSE  
    if (ok != true)
      printf("tx failed.\n\r");
#endif //VERBOSE
	usleep(delay_us);
}

int main(int argc, char** argv)
{
	printf("0)%s  1)%s  2)%s\n\r", argv[0], argv[1], argv[2]);
	if (argc > 1) {
		autobroadcast = false;
		sscanf(argv[1],"%lud",&delay_ms);  //read in milliseconds
		delay_us = delay_ms * 1000;
	}
        setup();
        while(1)
                loop();
        return 0;
}
