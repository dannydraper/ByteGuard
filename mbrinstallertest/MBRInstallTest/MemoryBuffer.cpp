// A memory buffer class. Written by Danny Draper (c) 2007. Provides safe memory buffer storage.

#include "MemoryBuffer.h"

MemoryBuffer::MemoryBuffer ()
{
	m_bCleared = true;
	m_bSizeset = false;
	m_iBuffersize = 0;
	m_iAppendpointer = 0;
}

MemoryBuffer::~MemoryBuffer ()
{
}



void MemoryBuffer::SetSize (unsigned long iSize)
{
	if (m_bSizeset == false) {
		m_bSizeset = true;
		m_bCleared = false;
		m_buffer = (BYTE *) calloc (iSize, 1);
		m_iBuffersize = iSize;
	} else {
		m_buffer = (BYTE *) realloc (m_buffer, iSize);
		m_iBuffersize = iSize;
		m_iAppendpointer = 0;
		m_bCleared = false;
		ZeroMemory (m_buffer, iSize);
	}
}

unsigned long MemoryBuffer::GetSize ()
{
	return m_iBuffersize;
}

BYTE MemoryBuffer::GetByte (unsigned long lIndex)
{
	if (m_bSizeset == true) {
		if (m_iAppendpointer > 0) {
			if (lIndex >=0 && lIndex < m_iAppendpointer) {
				return m_buffer[lIndex];
			} else {
				return 0;
			}
		} else {
			if (lIndex >=0 && lIndex < m_iBuffersize) {
				return m_buffer[lIndex];
			} else {
				return 0;
			}
		}
	} else {
		return 0;
	}
}

void MemoryBuffer::SetByte (unsigned long lIndex, BYTE bByte)
{
	if (m_bSizeset == true) {
		if (m_iAppendpointer > 0) {
			if (lIndex >=0 && lIndex < m_iAppendpointer) {
				m_buffer[lIndex] = bByte;
			}
		} else {
			if (lIndex >=0 && lIndex < m_iBuffersize) {
				m_buffer[lIndex] = bByte;
			}
		}
	}
}

bool MemoryBuffer::Write (void *pItem, unsigned long iOffset, unsigned long itemSize)
{
	if (m_bSizeset == true) { // Make sure memory is allocated
		if (iOffset >=0 && iOffset < m_iBuffersize) { // Make sure the offset is within bounds.
			if (itemSize <= (m_iBuffersize-iOffset)) { // Make sure the item size will fit within the space available
				memcpy (m_buffer+iOffset, pItem, itemSize);
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool MemoryBuffer::Append (void *pItem, unsigned long itemSize)
{
	if (m_bSizeset == true) { // Make sure memory is allocated
		if (itemSize <= (m_iBuffersize-m_iAppendpointer)) {
			memcpy (m_buffer+m_iAppendpointer, pItem, itemSize);
			m_iAppendpointer+=itemSize;
			return true;
		} else {
			return false;
		}
	} else {
		return false;	
	}
}

bool MemoryBuffer::SaveToFile (LPCSTR lpszFilename)
{
	FILE *stream;

	if (m_bSizeset == true) {
		if (m_iAppendpointer > 0) {
			if ((stream = fopen (lpszFilename, "w+b")) == NULL) {
				_fcloseall ();
				return false;
			} else {
				fwrite (m_buffer, m_iAppendpointer, 1, stream);
				_fcloseall ();
				return true;
			}			
		} else {
			if ((stream = fopen (lpszFilename, "w+b")) == NULL) {
				_fcloseall ();
				return false;
			} else {
				fwrite (m_buffer, m_iBuffersize, 1, stream);
				_fcloseall ();
				return true;
			}

		}
	} else {
		return false;
	}
}

int MemoryBuffer::ReadFromResource (LPCTSTR ResourceName) {
	HRSRC hResource;
	hResource = FindResource (GetModuleHandle (NULL), ResourceName, "BINARY");
	if (hResource == NULL) {		
		
		return -2;
	} else {
		HGLOBAL hLoad = LoadResource (GetModuleHandle (NULL), hResource);
		void *data = LockResource (hLoad);
		size_t sz = SizeofResource (GetModuleHandle (NULL), hResource);		
		SetSize (sz);
		Write (data, 0, sz);
		return 0;
	}
}



bool MemoryBuffer::SaveToClipboard (HWND hwnd)
{
	HANDLE hMem;
	LPBYTE lpbyteMem;
	LPTSTR  lptstrCopy;
	DWORD dwErr;

	if (m_bSizeset == true) {
		if (m_iAppendpointer > 0) {

			hMem = GlobalAlloc (GMEM_MOVEABLE, m_iAppendpointer);
			lptstrCopy = (LPTSTR) GlobalLock (hMem);
			memcpy (lptstrCopy, m_buffer, m_iAppendpointer);			
			GlobalUnlock (hMem);

		} else {
			hMem = GlobalAlloc (GMEM_MOVEABLE, m_iBuffersize);			    
			lptstrCopy = (LPTSTR) GlobalLock (hMem);
			memcpy (lptstrCopy, m_buffer, m_iBuffersize);
			GlobalUnlock (hMem);
		}

		// Now put the data in the clipboard
		if (OpenClipboard (NULL) != 0) {
			
			if (EmptyClipboard() == 0) {
				MessageBox (NULL, "EmptyClipboard Failed.", "Error", MB_OK);					
				return false;
			}

			if (SetClipboardData (CF_TEXT, hMem) == NULL) {
				dwErr = GetLastError ();
				MessageBox (NULL, "SetClipboardData Failed.", "Error", MB_OK);					
				return false;
			}

			if (CloseClipboard () == 0) {
				dwErr = GetLastError ();
				MessageBox (NULL, "CloseClipboard Failed.", "Error", MB_OK);					
				return false;
			}						

		} else {
			dwErr = GetLastError ();
			MessageBox (NULL, "OpenClipboard Failed.", "Error", MB_OK);								
			return false;
		}
		
		return true;
	} else {
		return false;
	}
}

bool MemoryBuffer::ReadFromFile (LPCSTR lpszFilename)
{
	FILE *stream2;
	int fh = 0;
	int filelen = 0;

	// Open a file
	if( _sopen_s(&fh, lpszFilename, _O_BINARY | _O_RDONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE ) == 0 )
	{
		filelen = _filelength( fh );
		_close( fh );

		if( (stream2 = fopen( lpszFilename, "rb" )) == NULL ) {
			_fcloseall( );
			

			errno_t err;			
			_get_errno( &err );
			
			char szErr[512];
			ZeroMemory (szErr, 512);

			sprintf_s(szErr, 512, "errno = %d\n", err );
			//sprintf_s(szErr, 512, "fyi, ENOENT = %d\n", ENOENT );			

			MessageBox (NULL, strerror (err), "Error", MB_OK);

			return false;
		} else {
			// Allocate the memory we need
			SetSize (filelen);

			fread( m_buffer, sizeof(BYTE), filelen, stream2);
			m_iAppendpointer = filelen;
			m_iBuffersize = filelen;

			_fcloseall( );
			return true;
		}
	    // All other files are closed:
	    _fcloseall( );
	} else {
		

		//MessageBox (NULL, "Second Error!", "Error", MB_OK);
		return false;
	}
}

bool MemoryBuffer::SwapBytes (unsigned long iIndex1, unsigned long iIndex2)
{
	BYTE bValue1;
	BYTE bValue2;

	if (m_bSizeset == true) {
		if (m_iAppendpointer > 0) {
			if (iIndex1 >= 0 && iIndex1 <=m_iAppendpointer) {
				if (iIndex2 >= 0 && iIndex2 <=m_iAppendpointer) {
					if (iIndex2 != iIndex1) {
						bValue1 = m_buffer[iIndex1];
						bValue2 = m_buffer[iIndex2];
						m_buffer[iIndex1] = bValue2;
						m_buffer[iIndex2] = bValue1;
						return true;
					} else {
						return false;
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			if (iIndex1 >= 0 && iIndex1 <=m_iBuffersize) {
				if (iIndex2 >= 0 && iIndex2 <=m_iBuffersize) {
					if (iIndex2 != iIndex1) {
						bValue1 = m_buffer[iIndex1];
						bValue2 = m_buffer[iIndex2];
						m_buffer[iIndex1] = bValue2;
						m_buffer[iIndex2] = bValue1;
						return true;
					} else {
						return false;
					}
				} else {
					return false;
				}
			} else {
				return false;
			}
		}
	} else {
		return false;
	}
}

void MemoryBuffer::QuickCrypt (bool Encrypt)
{
	// Build a really small and simple key
	int ikey[20];
	ikey[0] = 20;
	ikey[1] = 10;
	ikey[2] = 87;
	ikey[3] = 45;
	ikey[4] = 32;
	ikey[5] = 14;
	ikey[6] = 35;
	ikey[7] = 67;
	ikey[8] = 77;
	ikey[9] = 89;
	ikey[10] = 62;
	ikey[11] = 74;
	ikey[12] = 72;
	ikey[13] = 42;
	ikey[14] = 12;
	ikey[15] = 93;
	ikey[16] = 72;
	ikey[17] = 54;
	ikey[18] = 39;
	ikey[19] = 55;
	ikey[20] = 41;
	
	int ikeypointer = 0;
	unsigned long size = 0;
	unsigned long b = 0;
	BYTE bCurrent;

	if (m_bSizeset == true) {
		if (m_iAppendpointer > 0) {
			size = m_iAppendpointer;
		} else {
			size = m_iBuffersize;
		}

		for (b=0;b<size;b++) {
			bCurrent = GetByte (b);

			if (Encrypt == true) {
				bCurrent += ikey[ikeypointer];
			} else {
				bCurrent -= ikey[ikeypointer];
			}

			if (ikeypointer > 20) {
				ikeypointer = 0;
			}

			SetByte (b, bCurrent);
		}
	}
}

unsigned long MemoryBuffer::GetAppendPointer ()
{
	return m_iAppendpointer;
}

void *MemoryBuffer::GetBuffer ()
{
	return (void *) m_buffer;
}

bool MemoryBuffer::IsSizeset ()
{
	return m_bSizeset;
}

void MemoryBuffer::Clear ()
{
	if (m_bCleared == false) {
		m_bCleared = true;
		m_bSizeset = false;
		m_iBuffersize = 0;
		m_iAppendpointer = 0;
		// Free the buffer
		free (m_buffer);	
	}
}

/*
bool MemoryBuffer::StartSharing (char *szSharename, unsigned long lMaxsize)
{	
	// Gives the memory buffer a method of IPC (Inteprocess Communication) by using
	// Shared memory. Multiple process can access this shared memory by using the
	// sharing functions supplied here.

	// If a process is hosting the shared memory, it must call StartSharing and only then
	// can the second process read from it's shared memory.
	// Once the app hosting the shared memory is finished, it must call StopSharing.

	char szName[SIZE_STRING];
	ZeroMemory (szName, SIZE_STRING);
	strcpy_s (szName, SIZE_STRING, "Global\\");
	strcat_s (szName, SIZE_STRING, szSharename);

	m_hSharedMem = CreateFileMapping(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security 
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD) 
                 lMaxsize,                // maximum object size (low-order DWORD)  
                 szName);                 // name of mapping object
 
   if (m_hSharedMem == NULL) 
   {
	   m_lsharedmemsize = 0;
		return false;
   } else {
	   m_lsharedmemsize = lMaxsize;
		return true;
   }
}

bool MemoryBuffer::Share ()
{
	if (m_lsharedmemsize == 0) {
		return false;
	}

	int lheadersize = sizeof (unsigned long) *5;

	if (GetSize () <= (m_lsharedmemsize-lheadersize)) {
		
		// Shared memory structure
		// ulong | ulong | ulong | ulong | ulong | byte *
		// s1	 | s2	 | s3	 | s4	 | size	 | data

		unsigned long s1 = 147712;
		unsigned long s2 = 770477;
		unsigned long s3 = 555555;
		unsigned long s4 = 777777;

		unsigned long memsize = GetSize ();
		BYTE *pSharedbuf;

		pSharedbuf = (BYTE *) MapViewOfFile(m_hSharedMem,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,                   
                        0,                   
                        m_lsharedmemsize);           
 
	   if (pSharedbuf == NULL) 
	   { 
			return false;			
	   } else {
			unsigned long ipointer = 0;

			memcpy (pSharedbuf+ipointer, &s1, sizeof (unsigned long));
			ipointer+=sizeof (unsigned long);

			memcpy (pSharedbuf+ipointer, &s2, sizeof (unsigned long));
			ipointer+=sizeof (unsigned long);

			memcpy (pSharedbuf+ipointer, &s3, sizeof (unsigned long));
			ipointer+=sizeof (unsigned long);

			memcpy (pSharedbuf+ipointer, &s4, sizeof (unsigned long));
			ipointer+=sizeof (unsigned long);

			memcpy (pSharedbuf+ipointer, &memsize, sizeof (unsigned long));
			ipointer+=sizeof (unsigned long);

			memcpy (pSharedbuf+ipointer, GetBuffer(), memsize);
			
			UnmapViewOfFile(pSharedbuf);
			return true;
	   }
	} else {
		return false;
	}
}

bool MemoryBuffer::ReadSharedMemory (char *szSharename, unsigned long lMaxsize)
{
	BYTE *pBuf;
	HANDLE hMapFile;
	char szName[SIZE_STRING];
	ZeroMemory (szName, SIZE_STRING);
	strcpy_s (szName, SIZE_STRING, "Global\\");
	strcat_s (szName, SIZE_STRING, szSharename);

	hMapFile = OpenFileMapping(
				   FILE_MAP_ALL_ACCESS,   // read/write access
				   FALSE,                 // do not inherit the name
				   szName);               // name of mapping object 

	if (hMapFile == NULL) 
	{ 	  
		return false;
	} 

	
	pBuf = (BYTE *) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,                   
                        0,                   
                        lMaxsize);           
 
   if (pBuf == NULL) 
   { 
		return false;
   }

	
   // If we got here then we are reading check for the correct signature
   // and read the data into this memory buffer object.
	unsigned long o1 = 147712;
	unsigned long o2 = 770477;
	unsigned long o3 = 555555;
	unsigned long o4 = 777777;

	unsigned long s1;
	unsigned long s2;
	unsigned long s3;
	unsigned long s4;

	unsigned long datasize;

	unsigned long ipointer = 0;

	memcpy (&s1, (BYTE *) pBuf+ipointer, sizeof (unsigned long));
	ipointer+=sizeof (unsigned long);

	memcpy (&s2, (BYTE *) pBuf+ipointer, sizeof (unsigned long));
	ipointer+=sizeof (unsigned long);

	memcpy (&s3, (BYTE *) pBuf+ipointer, sizeof (unsigned long));
	ipointer+=sizeof (unsigned long);

	memcpy (&s4, (BYTE *) pBuf+ipointer, sizeof (unsigned long));
	ipointer+=sizeof (unsigned long);

	// Check the signature
	if (s1 != o1) {
		return false;
	}
	if (s2 != o2) {
		return false;
	}
	if (s3 != o3) {
		return false;
	}
	if (s4 != o4) {
		return false;
	}

	// If we got here then the signature matched what we are expecting and the contents in the shared
	// memory is definately something created by us. So now read the size of it
	
	memcpy (&datasize, (BYTE *) pBuf+ipointer, sizeof (unsigned long));
	ipointer+=sizeof (unsigned long);

	// Now set the memory buffer size and write the data
	Clear();
	if (datasize <= lMaxsize) {
		SetSize (datasize);
		Write (pBuf+ipointer, 0, datasize);
	} else {
		// Datasize is too large - it is not the size we are expecting so return false
		return false;
	}


   //CopyMemory((PVOID)pBuf, szMessage, (strlen(szMessage) * sizeof(TCHAR)));
   UnmapViewOfFile(pBuf);
   CloseHandle(hMapFile);

   return true; // If we got here then everything succeeded.
}

void MemoryBuffer::StopSharing ()
{
	m_lsharedmemsize = 0;
	CloseHandle(m_hSharedMem);
}
*/
