/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.6 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "TextReader.h"

namespace TrueCrypt
{
	TextReader::TextReader (const FilePath &path)
	{
		InputFile.reset (new File);
		InputFile->Open (path);
		InputStream = shared_ptr <Stream> (new FileStream (InputFile));
	}

	bool TextReader::ReadLine (string &outputString)
	{
		outputString.erase();

		char c;
		while (InputStream->Read (BufferPtr ((byte *) &c, sizeof (c))) == sizeof (c))
		{
			if (c == '\r')
				continue;

			if (c == '\n')
				return true;

			outputString += c;
		}
		return !outputString.empty();
	}
}
