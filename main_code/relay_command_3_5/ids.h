// updated 2014-08-21 20:18
// access digital values as bytes or bits (e.g. D1.byte, D1.relay.in2)
typedef union {
	struct {
		uint8_t in1:1;
		uint8_t in2:1;
		uint8_t in3:1;
		uint8_t in4:1;
		uint8_t in5:1;
		uint8_t in6:1;
		uint8_t in7:1;
		uint8_t in8:1;
	} relay;
	uint8_t byte;
//        boolean bits[8];
}  D_t;

//generic message
typedef struct {
	uint8_t id;
        uint8_t res[7];
} msg_GEN_t;

//packet id defs
#define ID_M  0x00  //full assignment
typedef struct {
	uint8_t id;	// message ID should be set to ID_M
        union {
          struct {
  	    D_t D1;	// Arduino #1 relay values for relays 1-8
  	    D_t D2;	// Arduino #2 relay values for relays 1-8
  	    D_t D3;	// Arduino #3 relay values for relays 1-8
          };
          D_t D[3];
        };
        union {
          struct {
	    uint8_t P1;	// Arduino #4 PWM value for SS switch #1
	    uint8_t P2;	// Arduino #4 PWM value for SS switch #2
	    uint8_t P3;	// Arduino #4 PWM value for SS switch #3
	    uint8_t P4;	// Arduino #4 PWM value for SS switch #4
          };
          uint8_t P[4];
        };
} msg_M_t;

#define ID_DN 0x10  //digital command to Arduino N
typedef struct {
	uint8_t id;	// message ID should be set to ID_DN
	uint8_t N;	// Arduino number
	D_t D1;		// Arduino #1 relay values for relays 1-8
	uint8_t res[5];	// reserved, set to zero
} msg_DN_t;

#define ID_D  0x20  //digital command to all binary relay Arduinos
typedef struct {
	uint8_t id;		// message ID should be set to ID_D
	union {			// allows access via array or name, e.g. D3 or D[3]
		struct {
			D_t D1;	// Arduino #1 relay values for relays 1-8
			D_t D2;	// Arduino #2 relay values for relays 1-8
			D_t D3;	// Arduino #3 relay values for relays 1-8
			D_t D4;	// Arduino #4 relay values for relays 1-8
			D_t D5;	// Arduino #5 relay values for relays 1-8
			D_t D6;	// Arduino #6 relay values for relays 1-8
			D_t D7;	// Arduino #7 relay values for relays 1-8
		};
		D_t D[7];	// Arduino 1-7 relay values for relays 1-8
	};
} msg_D_t;

#define ID_PN 0x30  // send command PWM command to Arduino N
typedef struct {
	uint8_t id;	// message ID should be set to ID_PN
	uint8_t N;	// Arduino number
        union {
          struct {
	    uint8_t P1;	// Arduino #4 PWM value for SS switch #1
	    uint8_t P2;	// Arduino #4 PWM value for SS switch #2
	    uint8_t P3;	// Arduino #4 PWM value for SS switch #3
	    uint8_t P4;	// Arduino #4 PWM value for SS switch #4
          };
          uint8_t P[4];
        };
	uint8_t res[2];	// reserved, set to zero
} msg_PN_t;

#define ID_S  0x40  //status request
typedef struct {
	uint8_t id;	// message ID should be set to ID_PN
	uint8_t N;	// Arduino number
	uint8_t res[6];	// reserved, set to zero
} msg_S_t;

#define ID_SR  0x50  //status response
typedef struct {
	uint8_t id;	// message ID should be set to ID_PN
	uint8_t N;	// Arduino number
	int8_t  temp;   // temperature in Celcius
	float humidity; // humidity percent
	uint8_t res[1];	// reserved, set to zero
} msg_SR_t;

//access a message as any message type
typedef union {
        void *raw;
        uint8_t bytes[8];
        msg_GEN_t msg_GEN;
        msg_M_t msg_M;
        msg_DN_t msg_DN;
        msg_D_t msg_D;
        msg_PN_t msg_PN;
        msg_S_t msg_S;
        msg_SR_t msg_SR;
} msg_t;
