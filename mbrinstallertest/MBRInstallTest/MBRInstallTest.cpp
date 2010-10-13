#include <Windows.h>
#include "MemoryBuffer.h"
#include "resource.h"

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	MessageBox (NULL, "Click OK to install the Bootloader - WARNING DO NOT RUN THIS ON A REAL COMPUTER!! VM TESTING ONLY!!!!", "Information", MB_OK);
	
	char szComputerName[SIZE_STRING];
	ZeroMemory (szComputerName, SIZE_STRING);

	DWORD dwSize = SIZE_STRING;

	GetComputerName (szComputerName, &dwSize);

	if (strcmp (szComputerName, "CYBERHAC-E7E840") != 0) {
		MessageBox (NULL, szComputerName, "Computer Name", MB_OK);
		MessageBox (NULL, "Safety Mechanism!!! This is not the intended testing VM!! Aborting!!!", "Safety Mechanism", MB_OK);
		return 0;
	}

	
				
	HANDLE hDisk = CreateFile ("\\\\.\\PhysicalDrive0", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDisk == INVALID_HANDLE_VALUE) {
		MessageBox (NULL, "Drive handle was invalid.", "Error", MB_OK);
		return 0;
	}

		
	long l = 0;	    
    SetFilePointer(hDisk, 0, &l, FILE_BEGIN); // 32-bit only limited to 4GB

	MemoryBuffer memTest;

	memTest.ReadFromResource (MAKEINTRESOURCE (IDB_BOOTLOADER));

	DWORD bytesWritten = 0;

	WriteFile (hDisk, memTest.GetBuffer(), memTest.GetSize (), &bytesWritten, NULL);

	
	CloseHandle (hDisk);

	
	
	MessageBox (NULL, "Done - Recommend you reboot.", "Information", MB_OK);
	
	
	return 0;


}