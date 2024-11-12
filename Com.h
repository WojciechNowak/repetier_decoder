#pragma once

#include <iostream>
#include <queue>

class Com
{
public:
	static void print(char p) {}
	static void print(const char* p) {}
	static void print(int p) {}
	static void printF(const char* command, float number) {}
	static void println() {}

private:
	std::queue<std::string> m_queue;
};