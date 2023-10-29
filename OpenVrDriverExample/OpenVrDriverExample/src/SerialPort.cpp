#include "SerialPort.h"

SerialPort::SerialPort(const char* portName) {
	// vr::VRDriverLog()->Log("Serial Port Entering...");
	this->connected = false;
	// vr::VRDriverLog()->Log("Create Handle");
	this->handler = CreateFileA(static_cast<LPCSTR>(portName), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (this->handler != INVALID_HANDLE_VALUE) {
		DCB dcbSerialParams = { 0 };
		if (GetCommState(this->handler, &dcbSerialParams)) {
			dcbSerialParams.BaudRate = CBR_115200;
			dcbSerialParams.ByteSize = 8;
			dcbSerialParams.StopBits = ONESTOPBIT;
			dcbSerialParams.Parity = NOPARITY;
			dcbSerialParams.fDtrControl = DTR_CONTROL_ENABLE;

			if (SetCommState(handler, &dcbSerialParams)) {
				this->connected = true;
				PurgeComm(this->handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
				// Sleep(5);
			}
		}

		// vr::VRDriverLog()->Log("Serial Port Exiting...");
	}
}

SerialPort::~SerialPort() {
	// vr::VRDriverLog()->Log("Serial Port Enter Successed!");
	if (this->connected) {
		this->connected = false;
		CloseHandle(this->handler);
	}
}

int SerialPort::readSerialPort(const char* buffer, unsigned int buf_size) {
	vr::VRDriverLog()->Log("SerialPort::readSerialPort 진입성공"); //this is how you log out Steam's log file.
	DWORD bytesRead{};
	unsigned int toRead = 0;

	ClearCommError(this->handler, &this->errors, &this->status);

	if (this->status.cbInQue > 0)
	{
		if (this->status.cbInQue > buf_size)
		{
			toRead = buf_size;
		}
		else
		{
			toRead = this->status.cbInQue;
		}
	}

	memset((void*)buffer, 0, buf_size);
	vr::VRDriverLog()->Log("SerialPort::readSerialPort 빠져나감"); //this is how you log out Steam's log file.
	if (ReadFile(this->handler, (void*)buffer, toRead, &bytesRead, NULL))
	{
		return bytesRead;
	}

	return 0;
}

bool SerialPort::isConnected()
{
	vr::VRDriverLog()->Log("SerialPort::isConnected 진입"); //this is how you log out Steam's log file.


	vr::VRDriverLog()->Log((const char*)(this->handler == NULL));

	vr::VRDriverLog()->Log((const char*)(this->errors == NULL));

	HANDLE a = this->handler;

	DWORD b = this->errors;

	COMSTAT c = this->status;

	vr::VRDriverLog()->Log((const char*)(ClearCommError(this->handler, &this->errors, &this->status)));

	if (!ClearCommError(this->handler, &this->errors, &this->status)) {
		this->connected = false;
	}
	return this->connected;
}

void SerialPort::closeSerial()
{
	vr::VRDriverLog()->Log("SerialPort::closeSerial 진입"); //this is how you log out Steam's log file.
	CloseHandle(this->handler);
	vr::VRDriverLog()->Log("SerialPort::closeSerial 탈출"); //this is how you log out Steam's log file.
}
