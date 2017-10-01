#include "SIMComAT.h"

void SIMComAT::begin(Stream& port)
{
	_port = &port;
	init();
}

void SIMComAT::flushInput()
{
	while (available()) read();
}

void SIMComAT::send() 
{
	flushInput();
	_port->println();
}

size_t SIMComAT::readLine(uint16_t timeout)
{
	uint8_t i = 0;

	while (timeout-- && i < BUFFER_SIZE)
	{
		while (available())
		{
			char c = read();
			if (c == '\r') continue;
			if (c == '\n')
			{
				if (i == 0) continue; //beginning of a new line
				else //end of the line
				{
					timeout = 0;
					break;
				}
			}
			_replyBuffer[i] = c;
			i++;
		}

		delay(1);
	}

	_replyBuffer[i] = 0; //string term

	RECEIVEARROW;
	PRINTLN(_replyBuffer);

	return strlen(_replyBuffer);
}

size_t SIMComAT::sendGetResponse(const char* msg, char* response, uint16_t timeout = SIMCOMAT_DEFAULT_TIMEOUT)
{
	SENDARROW;
	print(msg);
	return sendGetResponse(response, timeout);
}

size_t SIMComAT::sendGetResponse(char* response, uint16_t timeout = SIMCOMAT_DEFAULT_TIMEOUT) 
{
	send();
	readLine(timeout);
	
	size_t len = strlen(_replyBuffer);
	if (response != NULL) {
		size_t maxLen = min(len + 1, BUFFER_SIZE - 1);

		strncpy(response, _replyBuffer, maxLen);
		response[maxLen] = '\0';
	}

	return len;
}

bool SIMComAT::sendAssertResponse(const char* msg, const char* expectedResponse, uint16_t timeout = SIMCOMAT_DEFAULT_TIMEOUT)
{
	if (!sendGetResponse(msg, NULL, timeout)) return false;
	return assertResponse(expectedResponse);
}

bool SIMComAT::sendAssertResponse(const char* expectedResponse, uint16_t timeout = SIMCOMAT_DEFAULT_TIMEOUT)
{
	if (!sendGetResponse(NULL, timeout)) return false;
	return assertResponse(expectedResponse);
}

bool SIMComAT::assertResponse(const char* expectedResponse)
{
	PRINT(F("assertResponse : ["));
	PRINT(_replyBuffer);
	PRINT("], [");
	PRINT(expectedResponse);
	PRINTLN("]");

	return !strcasecmp(_replyBuffer, expectedResponse);
}

bool SIMComAT::parseReply(char divider, uint8_t index, uint8_t* result) 
{
	uint16_t tmpResult;
	if (!parseReply(divider, index, &tmpResult)) return false;

	*result = (uint8_t)tmpResult;
	return true;
}

bool SIMComAT::parseReply(char divider, uint8_t index, uint16_t* result) 
{
	PRINT("parseReply : [");
	PRINT(divider);
	PRINT(", ");
	PRINT(index);
	PRINT(", ");
	PRINT(_replyBuffer);
	PRINTLN("]");

	const char* p = strchr(_replyBuffer, ':');
	if (p == NULL) return false;

	for (uint8_t i = 0; i < index; i++)
	{
		p = strchr(p, divider);
		if (p == NULL) return false;
		p++;
	}

	*result = atoi(p); //TODO : unsigned

	PRINT("parseReply : [");
	PRINT(divider);
	PRINT(", ");
	PRINT(index);
	PRINT(", ");
	PRINT(_replyBuffer);
	PRINT("], [");
	PRINT(*result);
	PRINTLN("]");

	return true;
}
