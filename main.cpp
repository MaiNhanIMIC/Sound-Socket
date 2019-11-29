#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include "stdint.h"


using namespace std;

#pragma comment(lib, "winmm.lib")
#pragma comment(lib,"ws2_32.lib") //Winsock Library
WSADATA wsa;
SOCKET s;
struct sockaddr_in server;
char* message;
int Socket_Init(char* ip_server, u_short port);
int Socket_Send(char* data, uint16_t size);
int Socket_Recv(char* src, uint16_t size);

short int waveIn[102400];

void PlaySound(uint16_t channelNum, uint32_t sampleRate, uint16_t BlockAlign, uint32_t BitsPerSample, uint16_t format);
void writedataTofile(LPSTR lpData, DWORD dwBufferLength);

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct 
{
	char chunkID[4];
	uint32_t fileSize;
	char fileType[4];
	char formatChunk[4];
	uint32_t headerLen;
	uint16_t formatType;
	uint16_t channelNum;
	uint32_t sampleRate;
	uint32_t byteRate;
	uint16_t blockAlign;
	uint16_t bitPerSample;
	char chunk1ID[4];
	uint32_t chunk1Size;
}wavHeader_t;

void StartRecord()
{
	const int NUMPTS = 8000 * 3; // 3 seconds
	int sampleRate = 8000;
	// 'short int' is a 16-bit type; I request 16-bit samples below
	// for 8-bit capture, you'd use 'unsigned char' or 'BYTE' 8-bit types

	HWAVEIN hWaveIn;
	MMRESULT result;

	WAVEFORMATEX pFormat;
	pFormat.wFormatTag = WAVE_FORMAT_PCM;													// simple, uncompressed format
	pFormat.nChannels = 1;																	// 1=mono, 2=stereo
	pFormat.nSamplesPerSec = sampleRate;													// 8.0 kHz, 11.025 kHz, 22.05 kHz, and 44.1 kHz
	pFormat.nAvgBytesPerSec = sampleRate * 2;												// = nSamplesPerSec × nBlockAlign
	pFormat.nBlockAlign = 2;																// = (nChannels × wBitsPerSample) / 8
	pFormat.wBitsPerSample = 16;															// 16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;

	// Specify recording parameters

	result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat,
		0L, 0L, WAVE_FORMAT_DIRECT);

	WAVEHDR WaveInHdr;
	// Set up and prepare header for input
	WaveInHdr.lpData = (LPSTR)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;
	waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

	// Insert a wave input buffer
	result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));


	// Commence sampling input
	result = waveInStart(hWaveIn);


	printf("recording...\r\n");

	Sleep(3 * 1000);
	// Wait until finished recording

	waveInClose(hWaveIn);
	//PlaySound();
}

void PlaySound(uint16_t channelNum, uint32_t sampleRate, uint16_t BlockAlign, uint32_t BitsPerSample, uint16_t format)
{
	const int NUMPTS = sampleRate * 3;													 // 3 seconds
	//int sampleRate = 8000;
	// 'short int' is a 16-bit type; I request 16-bit samples below
	// for 8-bit capture, you'd use 'unsigned char' or 'BYTE' 8-bit types

	HWAVEIN hWaveIn;

	WAVEFORMATEX pFormat;
	pFormat.wFormatTag = format;											// simple, uncompressed format
	pFormat.nChannels = channelNum;													// 1=mono, 2=stereo
	pFormat.nSamplesPerSec = sampleRate;											// 44100
	pFormat.nAvgBytesPerSec = sampleRate * 2;										// = nSamplesPerSec * n.Channels * wBitsPerSample/8
	pFormat.nBlockAlign = BlockAlign;												// = n.Channels * wBitsPerSample/8
	pFormat.wBitsPerSample = BitsPerSample;											// 16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;

	// Specify recording parameters

	waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);

	WAVEHDR WaveInHdr;
	// Set up and prepare header for input
	WaveInHdr.lpData = (LPSTR)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;
	waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

	HWAVEOUT hWaveOut;
	//printf("playing...\r\n");
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &pFormat, 0, 0, WAVE_FORMAT_DIRECT);
	waveOutWrite(hWaveOut, &WaveInHdr, sizeof(WaveInHdr));							// Playing the data
	//Sleep(sizeof(waveIn)*1000/(sampleRate* channelNum));

	waveInClose(hWaveIn);
	waveOutClose(hWaveOut);
}

int Socket_Init(char* ip_server, u_short port)
{
	printf(" \nInitialising Winsock... ");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf(" Failed.Error Code : % d ", WSAGetLastError());
		return 1;
	}

	printf(" Initialised.\n ");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : % d", WSAGetLastError());
	}

	printf(" Socket created.\n ");


	server.sin_addr.s_addr = inet_addr(ip_server);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	//Connect to remote server
	if (connect(s, (struct sockaddr*) & server, sizeof(server)), 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");
}

int Socket_Send(char* data, uint16_t size)
{
	return send(s, data, size, 0);
}

int Socket_Recv(char* src, uint16_t size)
{
	return recv(s, src, size, 0);
}

int main()
{
	//StartRecord();
	Socket_Init((char*)"192.168.0.112", 80);
	char* msg;
	msg = (char*)"GET /1.wav HTTP/1.1\r\nHost: 192.168.0.112\r\n\r\n";
	uint16_t byteSend = 0;
	Socket_Send(msg, strlen(msg));
	char header[512];
	bool headerRecv = false;
	int cnt = 0;
	memset(header, 0, sizeof(header));
	while (!headerRecv)
	{
		Socket_Recv((char*)(header + cnt++), 1);
		for ( int i = 0; i < cnt; i++)
		{
			if (strstr(header, "\r\n\r\n")!= NULL)
			{
				headerRecv = true;
			}
		}
	}
	printf("header: %s", header);

	uint8_t wavHeader_buf[44];
	Socket_Recv((char*)wavHeader_buf, sizeof(wavHeader_buf));
	
	for (int i = 0; i < sizeof(wavHeader_buf); i++)
	{
		printf("%02x ", wavHeader_buf[i]);
	}

	wavHeader_t wavHeader;
	memcpy(&wavHeader, wavHeader_buf, sizeof(wavHeader));
	printf("chunkID: %c%c%c%c\r\n", wavHeader.chunkID[0], wavHeader.chunkID[1], wavHeader.chunkID[2], wavHeader.chunkID[3]);
	printf("file size: %d\r\n", wavHeader.fileSize);
	printf("header length: %d\r\n", wavHeader.headerLen);
	printf("type of format: %d\r\n", wavHeader.formatType);
	printf("number of channel: %d\r\n", wavHeader.channelNum);
	printf("sample rate: %d\r\n", wavHeader.sampleRate);
	printf("byte rate: %d\r\n", wavHeader.byteRate);
	printf("block align: %d\r\n", wavHeader.blockAlign);
	printf("bit per sample: %d\r\n", wavHeader.bitPerSample);
	printf("chunk2Size: %ld\r\n", wavHeader.chunk1Size);

	while (1)
	{
		uint8_t buffer[1024];
		uint16_t byteRecv = 0;
		memset((void*)waveIn, 0, sizeof(waveIn));
		byteRecv = Socket_Recv((char*)waveIn, sizeof(waveIn));
		PlaySound(wavHeader.channelNum, wavHeader.sampleRate, wavHeader.blockAlign, wavHeader.bitPerSample, wavHeader.formatType);
	}
	return 0;
}