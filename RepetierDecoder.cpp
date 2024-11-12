// RepetierDecoder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdint>
#include "Communication.h"
#include "gcode.h"


const unsigned int MIN_CMD_SIZE = 5;
const unsigned int MAX_CMD_SIZE2 = 128;

int main()
{
    std::filesystem::path path("data.gco");
    if (std::filesystem::exists(path))
    {
        Com::initialize();
        std::cout << "File size: " << std::filesystem::file_size(path) << std::endl;
        uintmax_t fileSize = std::filesystem::file_size(path);
        std::ifstream file("data.gco", std::ios::in | std::ios::binary);

        uint8_t receivedCommand[MAX_CMD_SIZE2];
        while (fileSize > MIN_CMD_SIZE)
        {
            std::memset(receivedCommand, 0, MAX_CMD_SIZE2);
            file.read((char*)receivedCommand, MIN_CMD_SIZE);
            auto bytesRead = file.gcount();
            fileSize = fileSize - bytesRead;
            auto size = GCode::computeBinarySize((char*)receivedCommand);
            auto bytesToRead = size - bytesRead;
            if (size > MIN_CMD_SIZE && size <= MAX_CMD_SIZE2 && fileSize >= bytesToRead)
            {
                file.read((char*)(receivedCommand + bytesRead), bytesToRead );
                fileSize = fileSize - file.gcount();
            }

            GCode gcode;
            gcode.parseBinary(receivedCommand, size, false);
            gcode.echoCommand();
        }
    }
    /*
    file.read((char*)receivedCommand, 5);
    std::memset(receivedCommand, 0, 5);
    
    while (file >> receivedCommand[0])
    {
        file >> receivedCommand[1];
        file >> receivedCommand[2];
        file >> receivedCommand[3];
        file >> receivedCommand[4];
        auto size = GCode::computeBinarySize((char*)receivedCommand);
    }*/
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
