/*
	This file is part of the Repetier-Firmware for RF devices from Conrad Electronic SE.

	Repetier-Firmware is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Repetier-Firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Repetier-Firmware.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "types.h"
#include <iostream>
#include <fstream>
#include <string>

class Com
{
public:
	FSTRINGVAR(tDebug)
		FSTRINGVAR(tFirmware)
		FSTRINGVAR(tOk)
		FSTRINGVAR(tNewline)
		FSTRINGVAR(tNAN)
		FSTRINGVAR(tINF)
		FSTRINGVAR(tError)
		FSTRINGVAR(tInfo)
		FSTRINGVAR(tWarning)
		FSTRINGVAR(tResend)
		FSTRINGVAR(tEcho)
		FSTRINGVAR(tOkSpace)
		FSTRINGVAR(tWrongChecksum)
		FSTRINGVAR(tMissingChecksum)
		FSTRINGVAR(tFormatError)
		FSTRINGVAR(tDonePrinting)
		FSTRINGVAR(tX)
		FSTRINGVAR(tY)
		FSTRINGVAR(tZ)
		FSTRINGVAR(tE)
		FSTRINGVAR(tF)
		FSTRINGVAR(tS)
		FSTRINGVAR(tP)
		FSTRINGVAR(tI)
		FSTRINGVAR(tJ)
		FSTRINGVAR(tR)
		FSTRINGVAR(tSDReadError)
		FSTRINGVAR(tExpectedLine)
		FSTRINGVAR(tGot)
		FSTRINGVAR(tSkip)
		FSTRINGVAR(tBLK)
		FSTRINGVAR(tStart)
		FSTRINGVAR(tStartWatchdog)
		FSTRINGVAR(tPowerUp)
		FSTRINGVAR(tExternalReset)
		FSTRINGVAR(tBrownOut)
		FSTRINGVAR(tWatchdog)
		FSTRINGVAR(tSoftwareReset)
		FSTRINGVAR(tUnknownCommand)
		FSTRINGVAR(tFreeRAM)
		FSTRINGVAR(tXColon)
		FSTRINGVAR(tSlash)
		FSTRINGVAR(tSpaceSlash)
		FSTRINGVAR(tSpaceXColon)
		FSTRINGVAR(tSpaceYColon)
		FSTRINGVAR(tSpaceZColon)
		FSTRINGVAR(tSpaceEColon)
		FSTRINGVAR(tTColon)
		FSTRINGVAR(tSpaceBColon)
		FSTRINGVAR(tSpaceAtColon)
		FSTRINGVAR(tSpaceT)
		FSTRINGVAR(tSpaceRaw)
		FSTRINGVAR(tSpaceAt)
		FSTRINGVAR(tSpaceBAtColon)
		FSTRINGVAR(tColon)
		FSTRINGVAR(tSpeedMultiply);
	FSTRINGVAR(tFlowMultiply);
	FSTRINGVAR(tFanspeed);
	FSTRINGVAR(tPrintedFilament)
		FSTRINGVAR(tPrintingTime)
		FSTRINGVAR(tMillingTime)
		FSTRINGVAR(tSpacem)
		FSTRINGVAR(tSpaceDaysSpace)
		FSTRINGVAR(tSpaceHoursSpace)
		FSTRINGVAR(tSpaceMin)
		FSTRINGVAR(tInvalidArc)
		FSTRINGVAR(tComma)
		FSTRINGVAR(tSpace)
		FSTRINGVAR(tYColon)
		FSTRINGVAR(tZColon)
		FSTRINGVAR(tE0Colon)
		FSTRINGVAR(tE1Colon)
		FSTRINGVAR(tMS1MS2Pins)
		FSTRINGVAR(tSetOutputSpace)
		FSTRINGVAR(tSpaceToSpace)
		FSTRINGVAR(tHSpace)
		FSTRINGVAR(tLSpace)
		FSTRINGVAR(tXMinColon)
		FSTRINGVAR(tXMaxColon)
		FSTRINGVAR(tYMinColon)
		FSTRINGVAR(tYMaxColon)
		FSTRINGVAR(tZMinColon)
		FSTRINGVAR(tZMaxColon)
		FSTRINGVAR(tJerkColon)
		FSTRINGVAR(tZJerkColon)
		FSTRINGVAR(tLinearStepsColon)
		FSTRINGVAR(tQuadraticStepsColon)
		FSTRINGVAR(tCommaSpeedEqual)
		FSTRINGVAR(tLinearLColon)
		FSTRINGVAR(tQuadraticKColon)
		FSTRINGVAR(tEEPROMUpdated)

#if FEATURE_SERVICE_INTERVAL
		FSTRINGVAR(tPrintedFilamentService)
		FSTRINGVAR(tPrintingTimeService)
		FSTRINGVAR(tMillingTimeService)
#endif // FEATURE_SERVICE_INTERVAL

#ifdef DEBUG_GENERIC
		FSTRINGVAR(tGenTemp)
#endif // DEBUG_GENERIC

		FSTRINGVAR(tTargetExtr)
		FSTRINGVAR(tTargetBedColon)
		FSTRINGVAR(tPIDAutotuneStart)
		FSTRINGVAR(tAPIDBias)
		FSTRINGVAR(tAPIDD)
		FSTRINGVAR(tAPIDMin)
		FSTRINGVAR(tAPIDMax)
		FSTRINGVAR(tAPIDKu)
		FSTRINGVAR(tAPIDTu)
		FSTRINGVAR(tAPIDClassic)
		FSTRINGVAR(tAPIDKp)
		FSTRINGVAR(tAPIDKi)
		FSTRINGVAR(tAPIDKd)
		FSTRINGVAR(tAPIDFailedHigh)
		FSTRINGVAR(tAPIDFailedTimeout)
		FSTRINGVAR(tAPIDFinished)
		FSTRINGVAR(tMTEMPColon)
		FSTRINGVAR(tHeatedBed)
		FSTRINGVAR(tExtruderSpace)
		FSTRINGVAR(tTempSensorDefect)
		FSTRINGVAR(tTempSensorWorking)
		FSTRINGVAR(tDryModeUntilRestart)

#if (DEBUG_QUEUE_MOVE || DEBUG_DIRECT_MOVE)
		FSTRINGVAR(tDBGId)
		FSTRINGVAR(tDBGVStartEnd)
		FSTRINGVAR(tDBAccelSteps)
		FSTRINGVAR(tDBGStartEndSpeed)
		FSTRINGVAR(tDBGFlags)
		FSTRINGVAR(tDBGJoinFlags)
		FSTRINGVAR(tDBGDelta)
		FSTRINGVAR(tDBGDir)
		FSTRINGVAR(tDBGFullSpeed)
		FSTRINGVAR(tDBGVMax)
		FSTRINGVAR(tDBGAcceleration)
		FSTRINGVAR(tDBGAccelerationPrim)
		FSTRINGVAR(tDBGRemainingSteps)
		FSTRINGVAR(tDBGAdvanceFull)
		FSTRINGVAR(tDBGAdvanceRate)
		FSTRINGVAR(tDBGLimitInterval)
		FSTRINGVAR(tDBGMoveDistance)
		FSTRINGVAR(tDBGCommandedFeedrate)
		FSTRINGVAR(tDBGConstFullSpeedMoveTime)
#endif // DEBUG_QUEUE_MOVE || DEBUG_DIRECT_MOVE

#ifdef DEBUG_SPLIT
		FSTRINGVAR(tDBGDeltaSeconds)
		FSTRINGVAR(tDBGDeltaZDelta)
		FSTRINGVAR(tDBGDeltaSegments)
		FSTRINGVAR(tDBGDeltaNumLines)
		FSTRINGVAR(tDBGDeltaSegmentsPerLine)
		FSTRINGVAR(tDBGDeltaMaxDS)
		FSTRINGVAR(tDBGDeltaStepsPerSegment)
		FSTRINGVAR(tDBGDeltaVirtualAxisSteps)
#endif // DEBUG_SPLIT

#ifdef DEBUG_STEPCOUNT
		FSTRINGVAR(tDBGMissedSteps)
#endif // DEBUG_STEPCOUNT

#ifdef WAITING_IDENTIFIER
		FSTRINGVAR(tWait)
#endif // WAITING_IDENTIFIER

#if EEPROM_MODE==0
		FSTRINGVAR(tNoEEPROMSupport)
#else
		FSTRINGVAR(tConfigStoredEEPROM)
		FSTRINGVAR(tConfigLoadedEEPROM)
		FSTRINGVAR(tEPRConfigResetDefaults)
		FSTRINGVAR(tEPRProtocolChanged)
		FSTRINGVAR(tExtrDot)
		FSTRINGVAR(tEPR0)
		FSTRINGVAR(tEPR1)
		FSTRINGVAR(tEPR2)
		FSTRINGVAR(tEPR3)
		FSTRINGVAR(tEPRBaudrate)
		FSTRINGVAR(tEPRFilamentPrinted)
		FSTRINGVAR(tEPRFilamentPrintedService)
		FSTRINGVAR(tEPRPrinterActive)
		FSTRINGVAR(tEPRPrinterActiveService)
		FSTRINGVAR(tEPRMillerActive)
		FSTRINGVAR(tEPRMillerActiveService)
		FSTRINGVAR(tEPRMaxInactiveTime)
		FSTRINGVAR(tEPRStopAfterInactivty)
		FSTRINGVAR(tEPRMaxJerk)
		FSTRINGVAR(tEPRXHomePos)
		FSTRINGVAR(tEPRYHomePos)
		FSTRINGVAR(tEPRZHomePos)
		FSTRINGVAR(tEPRXMaxLength)
		FSTRINGVAR(tEPRYMaxLength)
		FSTRINGVAR(tEPRZMaxLength)
		FSTRINGVAR(tEPRXBacklash)
		FSTRINGVAR(tEPRYBacklash)
		FSTRINGVAR(tEPRZBacklash)
FSTRINGVAR(tEPRZAcceleration)
FSTRINGVAR(tEPRZTravelAcceleration)
FSTRINGVAR(tEPRZStepsPerMM)
FSTRINGVAR(tEPRZMaxFeedrate)
FSTRINGVAR(tEPRZHomingFeedrate)
FSTRINGVAR(tEPRMaxZJerk)
FSTRINGVAR(tEPRXStepsPerMM)
FSTRINGVAR(tEPRYStepsPerMM)
FSTRINGVAR(tEPRXMaxFeedrate)
FSTRINGVAR(tEPRYMaxFeedrate)
FSTRINGVAR(tEPRXHomingFeedrate)
FSTRINGVAR(tEPRYHomingFeedrate)
FSTRINGVAR(tEPRXAcceleration)
FSTRINGVAR(tEPRYAcceleration)
FSTRINGVAR(tEPRXTravelAcceleration)
FSTRINGVAR(tEPRYTravelAcceleration)
FSTRINGVAR(tEPROPSMode)
FSTRINGVAR(tEPROPSMoveAfter)
FSTRINGVAR(tEPROPSMinDistance)
FSTRINGVAR(tEPROPSRetractionLength)
FSTRINGVAR(tEPROPSRetractionBacklash)
FSTRINGVAR(tEPRBedHeatManager)
FSTRINGVAR(tEPRBedPIDDriveMax)
FSTRINGVAR(tEPRBedPIDDriveMin)
FSTRINGVAR(tEPRBedPGain)
FSTRINGVAR(tEPRBedIGain)
FSTRINGVAR(tEPRBedDGain)
FSTRINGVAR(tEPRBedPISMaxValue)
FSTRINGVAR(tEPRStepsPerMM)
FSTRINGVAR(tEPRMaxFeedrate)
FSTRINGVAR(tEPRStartFeedrate)
FSTRINGVAR(tEPRAcceleration)
FSTRINGVAR(tEPRHeatManager)
FSTRINGVAR(tEPRDriveMax)
FSTRINGVAR(tEPRDriveMin)
FSTRINGVAR(tEPRPGain)
FSTRINGVAR(tEPRIGain)
FSTRINGVAR(tEPRDGain)
FSTRINGVAR(tEPRPIDMaxValue)
FSTRINGVAR(tEPRXOffset)
FSTRINGVAR(tEPRYOffset)
FSTRINGVAR(tEPRZOffset)
FSTRINGVAR(tEPRZMode)
FSTRINGVAR(tEPRStabilizeTime)
FSTRINGVAR(tEPRRetractionWhenHeating)
FSTRINGVAR(tEPRDistanceRetractHeating)
FSTRINGVAR(tEPRExtruderCoolerSpeed)
FSTRINGVAR(tEPRAdvanceK)
FSTRINGVAR(tEPRAdvanceL)
FSTRINGVAR(tEPRBeeperMode)
FSTRINGVAR(tEPRCaseLightsMode)
FSTRINGVAR(tEPR230VOutputMode)
FSTRINGVAR(tEPROperatingMode)
FSTRINGVAR(tEPRZEndstopType)
FSTRINGVAR(tEPRHotendType)
FSTRINGVAR(tEPRMillerType)
FSTRINGVAR(tEPRRGBLightMode)
FSTRINGVAR(tEPRFET1Mode)
FSTRINGVAR(tEPRFET2Mode)
FSTRINGVAR(tEPRFET3Mode)
#endif // EEPROM_MODE==0

#if SDSUPPORT
FSTRINGVAR(tSDRemoved)
FSTRINGVAR(tSDInserted)
FSTRINGVAR(tSDInitFail)
FSTRINGVAR(tErrorWritingToFile)
FSTRINGVAR(tBeginFileList)
FSTRINGVAR(tEndFileList)
FSTRINGVAR(tFileOpened)
FSTRINGVAR(tSpaceSizeColon)
FSTRINGVAR(tFileSelected)
FSTRINGVAR(tFileOpenFailed)
FSTRINGVAR(tSDPrintingByte)
FSTRINGVAR(tNotSDPrinting)
FSTRINGVAR(tOpenFailedFile)
FSTRINGVAR(tWritingToFile)
FSTRINGVAR(tDoneSavingFile)
FSTRINGVAR(tFileDeleted)
FSTRINGVAR(tDeletionFailed)
FSTRINGVAR(tDirectoryCreated)
FSTRINGVAR(tCreationFailed)
FSTRINGVAR(tSDErrorCode)
#endif // SDSUPPORT

FSTRINGVAR(tOutputObjectPrint)
FSTRINGVAR(tOutputObjectMill)
FSTRINGVAR(tUnmountFilamentWithHeating)
FSTRINGVAR(tUnmountFilamentWithoutHeating)
FSTRINGVAR(tMountFilamentWithHeating)
FSTRINGVAR(tMountFilamentWithoutHeating)

#if FEATURE_FIND_Z_ORIGIN
FSTRINGVAR(tFindZOrigin)
#endif // FEATURE_FIND_Z_ORIGIN

#if FEATURE_TEST_STRAIN_GAUGE
FSTRINGVAR(tTestStrainGauge)
#endif // FEATURE_TEST_STRAIN_GAUGE

static std::ofstream m_fstream;

static void initialize()
{
	m_fstream.open("data_decoded.gcode", std::ios::out);
}

static void writeToFile(const std::string& text)
{
	//m_fstream << text;
	if (m_fstream.is_open())
	{
		m_fstream.write(text.c_str(), text.length());
	}
}
    static void printNumber(uint32_t n);
	static void printWarningF(FSTRINGPARAM(text));
	static void printInfoF(FSTRINGPARAM(text));
	static void printErrorF(FSTRINGPARAM(text));
	static void printWarningFLN(FSTRINGPARAM(text));
	static void printInfoFLN(FSTRINGPARAM(text));
	static void printErrorFLN(FSTRINGPARAM(text));
	static void printFLN(FSTRINGPARAM(text));
	static void printF(FSTRINGPARAM(text));
	static void printF(FSTRINGPARAM(text), int16_t value);
	static void printF(FSTRINGPARAM(text), const char* msg);
	static void printF(FSTRINGPARAM(text), int32_t value);
	static void printF(FSTRINGPARAM(text), uint32_t value);
	static void printF(FSTRINGPARAM(text), float value, uint8_t digits = 2);
	static void printFLN(FSTRINGPARAM(text), int value);
	//static void printFLN(FSTRINGPARAM(text), int32_t value);
	static void printFLN(FSTRINGPARAM(text), uint32_t value);
	static void printFLN(FSTRINGPARAM(text), const char* msg);
	static void printFLN(FSTRINGPARAM(text), float value, uint8_t digits = 2);
	static void printArrayFLN(FSTRINGPARAM(text), float* arr, uint8_t n = 4, uint8_t digits = 2);
	static void printArrayFLN(FSTRINGPARAM(text), long* arr, uint8_t n = 4);
	static void print(long value);
	static inline void print(uint32_t value) { printNumber(value); }
	static inline void print(int value) { std::cout << std::to_string(value); writeToFile(std::to_string(value)); }
	static void print(const char* text);
	static inline void print(char c) { std::cout << c; std::string s; s.push_back(c); writeToFile(s); }
	static void printFloat(float number, uint8_t digits);
	static inline void println() { std::cout << std::endl; writeToFile("\n"); }

}; // Com


#endif // COMMUNICATION_H
