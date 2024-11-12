#pragma once

#include "types.h"

#define MAX_CMD_SIZE 128

enum FirmwareState { NotBusy = 0, Processing, Paused, WaitHeater };

class GCode   // 52 uint8_ts per command needed
{
public:
    unsigned int	params;
    unsigned int	params2;
    unsigned int	N; // Line number
    unsigned int	M;
    unsigned int	G;
    float			X;
    float			Y;
    float			Z;
    float			E;
    float			F;
    uint8_t			T;
    long			S;
    long			P;
    float			I;
    float			J;
    float			R;
    char* text;


    inline bool hasM()
    {
        return ((params & 2) != 0);
    } // hasM

    inline bool hasN()
    {
        return ((params & 1) != 0);
    } // hasN

    inline bool hasG()
    {
        return ((params & 4) != 0);
    } // hasG

    inline void setX(char set)
    {
        if (set)	params |= 8;
        else		params &= ~8;
    } // setX

    inline bool hasX()
    {
        return ((params & 8) != 0);
    } // hasX

    inline void setY(char set)
    {
        if (set)	params |= 16;
        else		params &= ~16;
    } // setY

    inline bool hasY()
    {
        return ((params & 16) != 0);
    } // hasY

    inline void setZ(char set)
    {
        if (set)	params |= 32;
        else		params &= ~32;
    } // setZ

    inline bool hasZ()
    {
        return ((params & 32) != 0);
    } // hasZ

    inline bool hasNoXYZ()
    {
        return ((params & 56) == 0);
    } // hasNoXYZ

    inline bool hasE()
    {
        return ((params & 64) != 0);
    } // hasE

    inline bool hasF()
    {
        return ((params & 256) != 0);
    } // hasF

    inline bool hasT()
    {
        return ((params & 512) != 0);
    } // hasT

    inline bool hasS()
    {
        return ((params & 1024) != 0);
    } // hasS

    inline bool hasP()
    {
        return ((params & 2048) != 0);
    } // hasP

    inline bool isV2()
    {
        return ((params & 4096) != 0);
    } // isV2

    inline bool hasString()
    {
        return ((params & 32768) != 0);
    } // hasString

    inline bool hasI()
    {
        return ((params2 & 1) != 0);
    } // hasI

    inline bool hasJ()
    {
        return ((params2 & 2) != 0);
    } // hasJ

    inline bool hasR()
    {
        return ((params2 & 4) != 0);
    } // hasR

    inline long getS(long def)
    {
        return (hasS() ? S : def);
    } // getS

    inline long getP(long def)
    {
        return (hasP() ? P : def);
    } // getP

    inline void setFormatError()
    {
        params2 |= 32768;
    } // setFormatError

    inline bool hasFormatError()
    {
        return ((params2 & 32768) != 0);
    } // hasFormatError

    void printCommand();
    bool parseBinary(uint8_t* buffer, uint8_t size, bool fromSerial);
    bool parseAscii(char* line, bool fromSerial);
    void popCurrentCommand();
    void echoCommand();
    static GCode* peekCurrentCommand();
    static void readFromSerial();
    static void readFromSD();
    static void pushCommand();
    static uint8_t computeBinarySize(char* ptr);
    static void keepAlive(enum FirmwareState state);
    static uint32_t keepAliveInterval;


private:
    void checkAndPushCommand();
    static void requestResend();

    inline float parseFloatValue(char* s)
    {
        char* endPtr;
        float f = (strtod(s, &endPtr));
        if (s == endPtr) setFormatError();
        return f;
    } // parseFloatValue

    inline long parseLongValue(char* s)
    {
        char* endPtr;
        long l = (strtol(s, &endPtr, 10));
        if (s == endPtr) setFormatError();
        return l;
    } // parseLongValue

    static GCode commandsBuffered[GCODE_BUFFER_SIZE];	///< Buffer for received commands.
    static uint8_t bufferReadIndex;						///< Read position in gcode_buffer.
    static uint8_t bufferWriteIndex;					///< Write position in gcode_buffer.
    static uint8_t commandReceiving[MAX_CMD_SIZE];		///< Current received command.
    static uint8_t commandsReceivingWritePosition;		///< Writing position in gcode_transbuffer.
    static uint8_t sendAsBinary;						///< Flags the command as binary input.
    static uint8_t wasLastCommandReceivedAsBinary;		///< Was the last successful command in binary mode?
    static uint8_t commentDetected;						///< Flags true if we are reading the comment part of a command.
    static uint8_t binaryCommandSize;					///< Expected size of the incoming binary command.
    static bool waitUntilAllCommandsAreParsed;			///< Don't read until all commands are parsed. Needed if gcode_buffer is misused as storage for strings.
    static uint32_t lastLineNumber;						///< Last line number received.
    static uint32_t actLineNumber;						///< Line number of current command.
    static volatile uint8_t bufferLength;				///< Number of commands stored in gcode_buffer
    static millis_t timeOfLastDataPacket;				///< Time, when we got the last data packet. Used to detect missing uint8_ts.
    static uint8_t formatErrors;						///< Number of sequential format errors
    static millis_t lastBusySignal;						///< When was the last busy signal

public:
    static int8_t waitingForResend;						///< Waiting for line to be resend. -1 = no wait.

}; // GCode
