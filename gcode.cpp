﻿#include "Communication.h"
#include "gcode.h"

#ifndef FEATURE_CHECKSUM_FORCED
#define FEATURE_CHECKSUM_FORCED false
#endif

GCode    GCode::commandsBuffered[GCODE_BUFFER_SIZE]; ///< Buffer for received commands.
uint8_t  GCode::bufferReadIndex = 0; ///< Read position in gcode_buffer.
uint8_t  GCode::bufferWriteIndex = 0; ///< Write position in gcode_buffer.
uint8_t  GCode::commandReceiving[MAX_CMD_SIZE]; ///< Current received command.
uint8_t  GCode::commandsReceivingWritePosition = 0; ///< Writing position in gcode_transbuffer.
uint8_t  GCode::sendAsBinary; ///< Flags the command as binary input.
uint8_t  GCode::wasLastCommandReceivedAsBinary = 0; ///< Was the last successful command in binary mode?
uint8_t  GCode::commentDetected = false; ///< Flags true if we are reading the comment part of a command.
uint8_t  GCode::binaryCommandSize; ///< Expected size of the incoming binary command.
bool     GCode::waitUntilAllCommandsAreParsed = false; ///< Don't read until all commands are parsed. Needed if gcode_buffer is misused as storage for strings.
uint32_t GCode::lastLineNumber = 0; ///< Last line number received.
uint32_t GCode::actLineNumber; ///< Line number of current command.
int8_t   GCode::waitingForResend = -1; ///< Waiting for line to be resend. -1 = no wait.
volatile uint8_t GCode::bufferLength = 0; ///< Number of commands stored in gcode_buffer
millis_t GCode::timeOfLastDataPacket = 0; ///< Time, when we got the last data packet. Used to detect missing uint8_ts.
uint8_t  GCode::formatErrors = 0;
millis_t GCode::lastBusySignal = 0; ///< When was the last busy signal
//uint32_t GCode::keepAliveInterval = KEEP_ALIVE_INTERVAL;

/** \page Repetier-protocol

\section Introduction

The repetier-protocol was developed, to overcome some shortcommings in the standard
RepRap communication method, while remaining backward compatible. To use the improved
features of this protocal, you need a host which speaks it. On Windows the recommended
host software is Repetier-Host. It is developed in parallel to this firmware and supports
all implemented features.

\subsection Improvements

- With higher speeds, the serial connection is more likely to produce communication failures.
  The standard method is to transfer a checksum at the end of the line. This checksum is the
  XORd value of all characters send. The value is limited to a range between 0 and 127. It can
  not detect two identical missing characters or a wrong order. Therefore the new protocol
  uses Fletchers checksum, which overcomes these shortcommings.
- The new protocol send data in binary format. This reduces the data size to less then 50% and
  it speeds up decoding the command. No slow conversion from string to floats are needed.

*/

/** \brief Computes size of binary data from bitfield.

In the repetier-protocol in binary mode, the first 2 uint8_ts define the
data. From this bitfield, this function computes the size of the command
including the 2 uint8_ts of the bitfield and the 2 uint8_ts for the checksum.

Gcode Letter to Bit and Datatype:

- N : Bit 0 : 16-Bit Integer
- M : Bit 1 :  8-Bit unsigned uint8_t
- G : Bit 2 :  8-Bit unsigned uint8_t
- X : Bit 3 :  32-Bit Float
- Y : Bit 4 :  32-Bit Float
- Z : Bit 5 :  32-Bit Float
- E : Bit 6 :  32-Bit Float
-  : Bit 7 :  always set to distinguish binary from ASCII line.
- F : Bit 8 :  32-Bit Float
- T : Bit 9 :  8 Bit Integer
- S : Bit 10 : 32 Bit Value
- P : Bit 11 : 32 Bit Integer
- V2 : Bit 12 : Version 2 command for additional commands/sizes
- Ext : Bit 13 : There are 2 more uint8_ts following with Bits, only for future versions
- Int :Bit 14 : Marks it as internal command,
- Text : Bit 15 : 16 Byte ASCII String terminated with 0
Second word if V2:
- I : Bit 0 : 32-Bit float
- J : Bit 1 : 32-Bit float
- R : Bit 2 : 32-Bit float
*/
uint8_t GCode::computeBinarySize(char* ptr)  // unsigned int bitfield) {
{
    uint8_t s = 4; // include checksum and bitfield
    uint16_t bitfield = *(uint16_t*)ptr;
    if (bitfield & 1) s += 2;
    if (bitfield & 8) s += 4;
    if (bitfield & 16) s += 4;
    if (bitfield & 32) s += 4;
    if (bitfield & 64) s += 4;
    if (bitfield & 256) s += 4;
    if (bitfield & 512) s += 1;
    if (bitfield & 1024) s += 4;
    if (bitfield & 2048) s += 4;
    if (bitfield & 4096)   // Version 2 or later
    {
        s += 2; // for bitfield 2
        uint16_t bitfield2 = *(uint16_t*)(ptr + 2);
        if (bitfield & 2) s += 2;
        if (bitfield & 4) s += 2;
        if (bitfield2 & 1) s += 4;
        if (bitfield2 & 2) s += 4;
        if (bitfield2 & 4) s += 4;
        if (bitfield & 32768) s += std::min(80, (uint8_t)ptr[4] + 1);
    }
    else
    {
        if (bitfield & 2) s += 1;
        if (bitfield & 4) s += 1;
        if (bitfield & 32768) s += 16;
    }
    return s;

} // computeBinarySize


void GCode::keepAlive(enum FirmwareState state)
{
} // keepAlive


void GCode::requestResend()
{
} // requestResend


/** \brief Check if result is plausible. If it is, an ok is send and the command is stored in queue.
    If not, a resend and ok is send. */
void GCode::checkAndPushCommand()
{
    if (hasM())
    {
        if (M == 110)   // Reset line number
        {
            lastLineNumber = actLineNumber;
            Com::printFLN(Com::tOk);
            waitingForResend = -1;
            return;
        }
        if (M == 112)   // Emergency kill - freeze printer
        {
            //Commands::emergencyStop();
        }
#ifdef DEBUG_COM_ERRORS
        if (M == 666)
        {
            lastLineNumber++;
            return;
        }
#endif // DEBUG_COM_ERRORS
    }
    if (hasN())
    {
        if (((lastLineNumber + 1) & 0xffff) != (actLineNumber & 0xffff))
        {
            if (waitingForResend < 0)   // after a resend, we have to skip the garbage in buffers, no message for this
            {
                    Com::printF(Com::tExpectedLine, lastLineNumber + 1);
                    Com::printFLN(Com::tGot, actLineNumber);
                requestResend(); // Line missing, force resend
            }
            else
            {
                --waitingForResend;
                commandsReceivingWritePosition = 0;
                Com::printFLN(Com::tSkip, actLineNumber);
                Com::printFLN(Com::tOk);
            }
            return;
        }
        lastLineNumber = actLineNumber;
    }
    pushCommand();

} // checkAndPushCommand


void GCode::pushCommand()
{
    bufferWriteIndex = (bufferWriteIndex + 1) % GCODE_BUFFER_SIZE;
    bufferLength++;
} // pushCommand


/** \brief Get the next buffered command. Returns 0 if no more commands are buffered. For each
    returned command, the popCurrentCommand() function must be called. */
GCode* GCode::peekCurrentCommand()
{
    if (bufferLength == 0) return NULL; // No more data
    return &commandsBuffered[bufferReadIndex];

} // peekCurrentCommand


/** \brief Removes the last returned command from cache. */
void GCode::popCurrentCommand()
{
#if DEBUG_COMMAND_PEEK
    Com::printFLN(PSTR("popCurrentCommand(): pop"));
#endif // DEBUG_COMMAND_PEEK

    if (!bufferLength) return; // Should not happen, but safety first

#ifdef ECHO_ON_EXECUTE
    echoCommand();
#endif // ECHO_ON_EXECUTE

    if (++bufferReadIndex == GCODE_BUFFER_SIZE) bufferReadIndex = 0;
    bufferLength--;

} // popCurrentCommand


void GCode::echoCommand()
{
	printCommand();
} // echoCommand

/** \brief Read from serial console
    This function is the main function to read the commands from serial console.
    It must be called frequently to empty the incoming buffer. */
void GCode::readFromSerial()
{
    if (bufferLength >= GCODE_BUFFER_SIZE || (waitUntilAllCommandsAreParsed && bufferLength))
    {
        // all buffers full
        return;
    }

    waitUntilAllCommandsAreParsed = false;
    while (commandsReceivingWritePosition < MAX_CMD_SIZE)    // consume data until no data or buffer full
    {

        if (!commandsReceivingWritePosition)
        {
            memset(commandReceiving, 0, sizeof(commandReceiving));
        }
        commandReceiving[commandsReceivingWritePosition++] = 0;//TODO get byte

        // first lets detect, if we got an old type ascii command
        if (commandsReceivingWritePosition == 1)
        {
            if (waitingForResend >= 0 && wasLastCommandReceivedAsBinary)
            {
                if (!commandReceiving[0])
                    waitingForResend--;   // Skip 30 zeros to get in sync
                else
                    waitingForResend = 30;
                commandsReceivingWritePosition = 0;
                continue;
            }
            if (!commandReceiving[0]) // Ignore zeros
            {
                commandsReceivingWritePosition = 0;
                continue;
            }
            sendAsBinary = (commandReceiving[0] & 128) != 0;
        }
        if (sendAsBinary)
        {
            if (commandsReceivingWritePosition < 2) continue;
            if (commandsReceivingWritePosition == 5 || commandsReceivingWritePosition == 4)
                binaryCommandSize = computeBinarySize((char*)commandReceiving);
            if (commandsReceivingWritePosition == binaryCommandSize)
            {
                GCode* act = &commandsBuffered[bufferWriteIndex];
                if (act->parseBinary(commandReceiving, binaryCommandSize, true))
                {
                    // Success
                    act->checkAndPushCommand();
                    //Com::printFLN(PSTR("Current binary from serial: "));
                    //act->printCommand();
                }
                else
                {
                    if (GCode::formatErrors < 3)
                    {
                        requestResend();
                    }
                    else
                    {
                        // we have to give up
#ifdef ACK_WITH_LINENUMBER
                        Com::printFLN(Com::tOkSpace, actLineNumber);
#else
                        Com::printFLN(Com::tOk);
#endif // ACK_WITH_LINENUMBER

                        waitingForResend = -1; // everything is (quasi) ok
                        lastLineNumber++;
                    }
                }
                commandsReceivingWritePosition = 0;
                return;
            }
        }
        else     // Ascii command
        {
            char ch = commandReceiving[commandsReceivingWritePosition - 1];
            if (ch == 0 || ch == '\n' || ch == '\r' || (!commentDetected && ch == ':'))  // complete line read
            {
                /*				Com::printF(PSTR("Parse serial ascii >>>"));
                                Com::print((char*)commandReceiving);
                                Com::printFLN(PSTR("<<<"));
                */
                commandReceiving[commandsReceivingWritePosition - 1] = 0;
                commentDetected = false;
                if (commandsReceivingWritePosition == 1)   // empty line ignore
                {
                    commandsReceivingWritePosition = 0;
                    continue;
                }
                GCode* act = &commandsBuffered[bufferWriteIndex];
                if (act->parseAscii((char*)commandReceiving, true))
                {
                    // Success
                    act->checkAndPushCommand();
                    //Com::printFLN(PSTR("Current ASCII from serial: "));
                    //act->printCommand();
                }
                else
                {
                    if (GCode::formatErrors < 3)
                    {
                        requestResend();
                    }
                    else
                    {
                        // we have to give up
#ifdef ACK_WITH_LINENUMBER
                        Com::printFLN(Com::tOkSpace, actLineNumber);
#else
                        Com::printFLN(Com::tOk);
#endif // ACK_WITH_LINENUMBER

                        waitingForResend = -1; // everything is (quasi) ok
                        lastLineNumber++;
                        formatErrors = 0;
                    }
                }
                commandsReceivingWritePosition = 0;
                return;
            }
            else
            {
                if (ch == ';') commentDetected = true; // ignore new data until lineend
                if (commentDetected) commandsReceivingWritePosition--;
            }
        }
        if (commandsReceivingWritePosition == MAX_CMD_SIZE)
        {
            requestResend();
            return;
        }
    }
    readFromSD();

} // readFromSerial


/** \brief Read from sdcard.
This function is the main function to read the commands from sdcard. */
void GCode::readFromSD()
{
#if SDSUPPORT
    if (!sd.sdmode || commandsReceivingWritePosition != 0)		// not reading or incoming serial command
        return;

    if (g_uBlockSDCommands)									// no further commands from the SD card shall be processed
        return;

    if (!PrintLine::checkForXFreeLines(2))
    {
        // we do not read G-Codes from the SD card until the cache is full -
        // when we leave a small space within the cache the PC is able to exchange commands faster (this effect is relevant only in case of G-Codes whose processing takes a noticeable amount of time, e.g. long and/or slow lines)
        return;
    }

    while (sd.filesize > sd.sdpos&& commandsReceivingWritePosition < MAX_CMD_SIZE)    // consume data until no data or buffer full
    {
        timeOfLastDataPacket = HAL::timeInMilliseconds();
        int n = sd.file.read();

        if (n == -1)
        {
            if (Printer::debugErrors())
            {
                Com::printFLN(Com::tSDReadError);
            }
            UI_ERROR("SD Read Error");

            // Second try in case of recoverable errors
            sd.file.seekSet(sd.sdpos);
            n = sd.file.read();

            if (n == -1)
            {
                if (Printer::debugErrors())
                {
                    Com::printErrorFLN(PSTR("SD error did not recover!"));
                }
                sd.sdmode = false;
                break;
            }
            UI_ERROR("SD error fixed");
        }
        sd.sdpos++; // = file.curPosition();
        commandReceiving[commandsReceivingWritePosition++] = (uint8_t)n;

        // first lets detect, if we got an old type ascii command
        if (commandsReceivingWritePosition == 1 && !commentDetected)
        {
            sendAsBinary = (commandReceiving[0] & 128) != 0;
        }
        if (sendAsBinary)
        {
            if (commandsReceivingWritePosition < 2) continue;
            if (commandsReceivingWritePosition == 4 || commandsReceivingWritePosition == 5)
                binaryCommandSize = computeBinarySize((char*)commandReceiving);
            if (commandsReceivingWritePosition == binaryCommandSize)
            {
                GCode* act = &commandsBuffered[bufferWriteIndex];
                if (act->parseBinary(commandReceiving, false))
                {
                    // Success, silently ignore illegal commands
                    pushCommand();
                    //                  Com::printF(PSTR("Current binary from SD: "));
                    //                  act->printCommand();
                }
                commandsReceivingWritePosition = 0;
                return;
            }
        }
        else
        {
            char ch = commandReceiving[commandsReceivingWritePosition - 1];
            bool returnChar = ch == '\n' || ch == '\r';
            if (returnChar || sd.filesize == sd.sdpos || (!commentDetected && ch == ':') || commandsReceivingWritePosition >= (MAX_CMD_SIZE - 1))  // complete line read
            {
                //Com::printF(PSTR("Parse SD ascii 1 >>>"));
                //Com::print((char*)commandReceiving);
                //Com::printFLN(PSTR("<<<"));

                if (returnChar || ch == ':')
                    commandReceiving[commandsReceivingWritePosition - 1] = 0;
                else
                    commandReceiving[commandsReceivingWritePosition] = 0;
                commentDetected = false;
                if (commandsReceivingWritePosition == 1)   // empty line ignore
                {
                    commandsReceivingWritePosition = 0;
                    memset(commandReceiving, 0, sizeof(commandReceiving));
                    continue;
                }

                //Com::printF(PSTR("Parse SD ascii 2 >>>"));
                //Com::print((char*)commandReceiving);
                //Com::printFLN(PSTR("<<<"));

                GCode* act = &commandsBuffered[bufferWriteIndex];
                if (act->parseAscii((char*)commandReceiving, false))
                {
                    // Success
                    pushCommand();
                    //Com::printF(PSTR("Current ASCII from SD: "));
                    //act->printCommand();
                }
                commandsReceivingWritePosition = 0;
                //memset( commandReceiving, 0, sizeof( commandReceiving ) );

                //Com::printF(PSTR("Verify: "));
                //act->printCommand();
                return;
            }
            else
            {
                /*				Com::printF(PSTR("Parse SD ascii 3 >>>"));
                                Com::print((char*)commandReceiving);
                                Com::printFLN(PSTR("<<<"));
                */
                if (ch == ';') commentDetected = true; // ignore new data until lineend
                if (commentDetected) commandsReceivingWritePosition--;
            }
        }
    }
    sd.sdmode = false;

    if (Printer::debugInfo())
    {
        Com::printFLN(Com::tDonePrinting);
    }
    commandsReceivingWritePosition = 0;
    commentDetected = false;
    Printer::setMenuMode(MENU_MODE_SD_PRINTING, false);

    BEEP_STOP_PRINTING
#endif // SDSUPPORT

} // readFromSD


/** \brief Converts a binary uint8_tfield containing one GCode line into a GCode structure.
    Returns true if checksum was correct. */
bool GCode::parseBinary(uint8_t* buffer, uint8_t size, bool fromSerial)
{
    unsigned int sum1 = 0, sum2 = 0; // for fletcher-16 checksum
    // first do fletcher-16 checksum tests see
    // http://en.wikipedia.org/wiki/Fletcher's_checksum
    uint8_t* p = buffer;
    uint8_t len = size - 2;


    while (len)
    {
        uint8_t tlen = len > 21 ? 21 : len;
        len -= tlen;
        do
        {
            sum1 += *p++;
            if (sum1 >= 255) sum1 -= 255;
            sum2 += sum1;
            if (sum2 >= 255) sum2 -= 255;
        } while (--tlen);
    }
    sum1 -= *p++;
    sum2 -= *p;
    if (sum1 | sum2)
    {
        {
            //            Com::printErrorFLN(Com::tWrongChecksum);

            Com::printErrorF(Com::tWrongChecksum);
            Com::printF(PSTR("|"), (int)sum1);
            Com::printF(PSTR("|"), (int)sum2);
            Com::printF(PSTR("|"));
        }
        return false;
    }

    p = buffer;
    params = *(unsigned int*)p;
    p += 2;
    uint8_t textlen = 16;
    if (isV2())
    {
        params2 = *(unsigned int*)p;
        p += 2;
        if (hasString())
            textlen = *p++;
    }
    else params2 = 0;

    if (params & 1)
    {
        actLineNumber = N = *(uint16_t*)p;
        p += 2;
    }
    if (isV2())   // Read G,M as 16 bit value
    {
        if (params & 2)
        {
            M = *(uint16_t*)p;
            p += 2;
        }
        if (params & 4)
        {
            G = *(uint16_t*)p;
            p += 2;
        }
    }
    else
    {
        if (params & 2)
        {
            M = *p++;
        }
        if (params & 4)
        {
            G = *p++;
        }
    }

    if (params & 8)
    {
        X = *(float*)p;
        p += 4;
    }
    if (params & 16)
    {
        Y = *(float*)p;
        p += 4;
    }
    if (params & 32)
    {
        Z = *(float*)p;
        p += 4;
    }
    if (params & 64)
    {
        E = *(float*)p;
        p += 4;
    }
    if (params & 256)
    {
        F = *(float*)p;
        p += 4;
    }
    if (params & 512)
    {
        T = *p++;
    }
    if (params & 1024)
    {
        S = *(int32_t*)p;
        p += 4;
    }
    if (params & 2048)
    {
        P = *(int32_t*)p;
        p += 4;
    }
    if (hasI())
    {
        I = *(float*)p;
        p += 4;
    }
    if (hasJ())
    {
        J = *(float*)p;
        p += 4;
    }
    if (hasR())
    {
        R = *(float*)p;
        p += 4;
    }
    if (hasString())   // set text pointer to string
    {
        text = (char*)p;
        text[textlen] = 0; // Terminate string overwriting checksum
        waitUntilAllCommandsAreParsed = true; // Don't destroy string until executed
    }
    return true;

} // parseBinary

bool GCode::parseAscii(char* line, bool fromSerial)
{
    bool has_checksum = false;
    char* pos;


    params = 0;
    params2 = 0;
    if ((pos = strchr(line, 'N')) != 0)   // Line number detected
    {
        actLineNumber = parseLongValue(++pos);
        params |= 1;
        N = actLineNumber & 0xffff;
    }

    if ((pos = strchr(line, 'M')) != 0)   // M command
    {
        M = parseLongValue(++pos) & 0xffff;
        params |= 2;
        if (M > 255) params |= 4096;
    }

    if (hasM() && (M == 23 || M == 28 || M == 29 || M == 30 || M == 32 || M == 117 || M == 3117))
    {
        // after M command we got a filename for sd card management
        char* sp = line;
        text = sp;

        while (*sp != 'M') sp++; // Search M command

        while (*sp != ' ')
        {
            // search next whitespace
            if (*sp == 0)
            {
                // end of string
                text = 0;
                break;
            }
            sp++;
        }

        while (*sp == ' ')
        {
            // skip leading whitespaces
            if (*sp == 0)
            {
                // end of string
                text = 0;
                break;
            }
            sp++;
        }

        if (text)
        {
            text = sp;
            while (*sp)
            {
                if ((M != 117 && M != 3117 && *sp == ' ') || *sp == '*') break; // end of filename reached
                sp++;
            }
            *sp = 0; // Removes checksum, but we don't care. Could also be part of the string.

            waitUntilAllCommandsAreParsed = true; // don't risk string be deleted
            params |= 32768;
        }
    }
    else
    {
        if ((pos = strchr(line, 'G')) != 0)   // G command
        {
            G = parseLongValue(++pos) & 0xffff;
            params |= 4;
            if (G > 255) params |= 4096;
        }
        if ((pos = strchr(line, 'X')) != 0)
        {
            X = parseFloatValue(++pos);
            params |= 8;
        }
        if ((pos = strchr(line, 'Y')) != 0)
        {
            Y = parseFloatValue(++pos);
            params |= 16;
        }
        if ((pos = strchr(line, 'Z')) != 0)
        {
            Z = parseFloatValue(++pos);
            params |= 32;
        }
        if ((pos = strchr(line, 'E')) != 0)
        {
            E = parseFloatValue(++pos);
            params |= 64;
        }
        if ((pos = strchr(line, 'F')) != 0)
        {
            F = parseFloatValue(++pos);
            params |= 256;
        }
        if ((pos = strchr(line, 'T')) != 0)   // M command
        {
            T = parseLongValue(++pos) & 0xff;
            params |= 512;
        }
        if ((pos = strchr(line, 'S')) != 0)   // M command
        {
            S = parseLongValue(++pos);
            params |= 1024;
        }
        if ((pos = strchr(line, 'P')) != 0)   // M command
        {
            P = parseLongValue(++pos);
            params |= 2048;
        }
        if ((pos = strchr(line, 'I')) != 0)
        {
            I = parseFloatValue(++pos);
            params2 |= 1;
            params |= 4096; // Needs V2 for saving
        }
        if ((pos = strchr(line, 'J')) != 0)
        {
            J = parseFloatValue(++pos);
            params2 |= 2;
            params |= 4096; // Needs V2 for saving
        }
        if ((pos = strchr(line, 'R')) != 0)
        {
            R = parseFloatValue(++pos);
            params2 |= 4;
            params |= 4096; // Needs V2 for saving
        }
    }

    if ((pos = strchr(line, '*')) != 0)   // checksum
    {
        uint8_t checksum_given = parseLongValue(pos + 1);
        uint8_t checksum = 0;
        while (line != pos) checksum ^= *line++;

#if FEATURE_CHECKSUM_FORCED
        Printer::flag0 |= PRINTER_FLAG0_FORCE_CHECKSUM;
#endif // FEATURE_CHECKSUM_FORCED

        if (checksum != checksum_given)
        {
            Com::printErrorFLN(Com::tWrongChecksum);
            return false; // mismatch
        }
    }

#if FEATURE_CHECKSUM_FORCED
    else
    {
        if (!fromSerial) return true;
        if (hasM() && (M == 110 || hasString())) return true;
        if (Printer::debugErrors())
        {
            Com::printErrorFLN(Com::tMissingChecksum);
        }
        return false;
    }
#endif // FEATURE_CHECKSUM_FORCED

    if (hasFormatError() || (params & 518) == 0)   // Must contain G, M or T command and parameter need to have variables!
    {
        formatErrors++;
            Com::printErrorFLN(Com::tFormatError);
            printCommand();

        return false;
    }
    else
    {
        formatErrors = 0;
    }
    return true;

} // parseAscii

/** \brief Print command on serial console */
void GCode::printCommand()
{
    if (hasN())
    {
        Com::print('N');
        Com::print((int)N);
        Com::print(' ');
    }
    if (hasM())
    {
        Com::print('M');
        Com::print((int)M);
        Com::print(' ');
    }
    if (hasG())
    {
        Com::print('G');
        Com::print((int)G);
        Com::print(' ');
    }
    if (hasT())
    {
        Com::print('T');
        Com::print((int)T);
        Com::print(' ');
    }
    if (hasX())
    {
        Com::printF(Com::tX, X);
    }
    if (hasY())
    {
        Com::printF(Com::tY, Y);
    }
    if (hasZ())
    {
        Com::printF(Com::tZ, Z);
    }
    if (hasE())
    {
        Com::printF(Com::tE, E, 4);
    }
    if (hasF())
    {
        Com::printF(Com::tF, F);
    }
    if (hasS())
    {
        Com::printF(Com::tS, (uint32_t)S);
    }
    if (hasP())
    {
        Com::printF(Com::tP, (uint32_t)P);
    }
    if (hasI())
    {
        Com::printF(Com::tI, I);
    }
    if (hasJ())
    {
        Com::printF(Com::tJ, J);
    }
    if (hasR())
    {
        Com::printF(Com::tR, R);
    }
    if (hasString())
    {
        Com::print(text);
    }
    Com::println();

} // printCommand
