// ************************************************************
// ** PROTOCOL HANDLER
// ************************************************************

#ifndef protocol_h
#define protocol_h


#define TOK_CMD 1
#define TOK_INT 2

#define CMD_LED 1
#define CMD_MOTOR 2
#define CMD_UPDATE 3

typedef enum
{
    S_IDLE = 0,
    S_INTEGER,
    S_CMD
} state_t;

typedef struct
{
    unsigned char bufferIdx;
    unsigned char buffer[5];
    state_t state;

    unsigned char tok_state;

    unsigned char args[8];  // up to 8 command arguments

} protocolState_t;

/** returns tokens: 0 for no new data, 1 for a command, 2 for integer */
unsigned char protoSubmitByte(protocolState_t *protocol, char c);

unsigned char protoSubmitToken(protocolState_t *protocol, unsigned char token);

#endif