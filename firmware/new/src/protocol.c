
#include <stdlib.h>
#include <stdio.h>
#include "protocol.h"

unsigned char isWhitespace(char c) // WhiteSpace or Separator
{
    if ((c==13)||(c==10)||(c==32)||(c==44)||(c==45)||(c==46)||(c==58)||(c==59)||(c=='\t'))
    {
        return 1;
    }
    return 0;
}

unsigned char isDigit(char c)
{
    if ((c >= '0') && (c <= '9'))
    {
        return 1;
    }
    return 0;
}

unsigned char isAlpha(char c)
{
    if ((c >= 'A') && (c <= 'Z')||(c >= 'a') && (c <= 'z'))
    {
        return 1;
    }
    return 0;    
}

unsigned char protoSubmitByte(protocolState_t *protocol, char c)
{
    switch(protocol->state)
    {
    case S_IDLE:
        protocol->bufferIdx = 0;
        if (!isWhitespace(c))
        {
            protocol->buffer[protocol->bufferIdx++] = c;
            if (isAlpha(c))
            {
                protocol->state = S_IDLE;
                protocol->buffer[protocol->bufferIdx] = 0;   // add null terminator
                return TOK_CMD; // emit command ID
            }
            else if (isDigit(c))
            {
                protocol->state = S_INTEGER;
                return 0;
            }
            return 0; // unrecognized char - ignore.
        }
        break;
    case S_INTEGER:
        if (isDigit(c))
        {
            protocol->buffer[protocol->bufferIdx] = c;
            if (protocol->bufferIdx < 3)
            {
                protocol->bufferIdx++;
            }
            return 0;
        }
        protocol->state = S_IDLE;
        protocol->buffer[protocol->bufferIdx] = 0;    // add null terminator
        return TOK_INT;   // emit integer ID
    default:
        protocol->state = S_IDLE;
    }
    return 0;
};


unsigned char protoSubmitToken(protocolState_t *protocol, unsigned char token)
{
    unsigned char ret = 0;

    // check for null-token and return
    if (token == 0)
    {
        return 0;
    }

    switch(protocol->tok_state)
    {
    case 0: // idle
        if (token == TOK_CMD) // command
        {
            unsigned char cmd = protocol->buffer[0];
            if (cmd == 'L'||cmd == 'l')
            {
                protocol->tok_state = 10;    // LED state
            }
            else if (cmd == 'M'||cmd == 'm')
            {
                protocol->tok_state = 20;    // Motor state
            }
            else if (cmd == 'U'||cmd == 'u')
            {
                // no arguments
                ret = CMD_UPDATE;
            }
            else if (cmd == 'R'||cmd == 'r')
            {
                // report motors, no arguments
                ret = CMD_REPORTMOTORS;
            }
            else if (cmd == 'T'||cmd == 't')
            {
                // report timer, no arguments
                ret = CMD_REPORTTIMER;
            }
            else if (cmd == 'H'||cmd == 'h')
            {
                // home motors, no arguments
                ret = CMD_HOMEMOTORS;
            }
        }
        break;
    case 10:   // LED n -LED number
        if (token == TOK_INT)
        {
            protocol->args[0] = atoi(protocol->buffer);
            protocol->tok_state = 11;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 11:   // LED r - LED red value
        if (token == TOK_INT)
        {
            protocol->args[1] = atoi(protocol->buffer);
            protocol->tok_state = 12;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 12:   // LED b - LED Blue value
        if (token == TOK_INT)
        {
            protocol->args[2] = atoi(protocol->buffer);
            protocol->tok_state = 13;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 13:   // LED g - LED Green Value
        if (token == TOK_INT)
        {
            protocol->args[3] = atoi(protocol->buffer);
            ret = CMD_LED;
        }
        protocol->tok_state = 0;
        break;
    case 20:   // Motor id (0 or 1)
        if (token == TOK_INT)
        {
            protocol->args[0] = atoi(protocol->buffer);
            protocol->tok_state = 21;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 21:   // Motor direction
        if (token == TOK_INT)
        {
            protocol->args[1] = atoi(protocol->buffer);
            protocol->tok_state = 22;
        }
        else
        {
            protocol->tok_state = 0; // error
        }
        break;
    case 22:   // Motor pulses
        if (token == TOK_INT)
        {
            protocol->args[2] = atoi(protocol->buffer);
            ret = CMD_MOTOR;
        }
        protocol->tok_state = 0;
        break;        
    default:
        protocol->tok_state = 0;
    }

    return ret;
}