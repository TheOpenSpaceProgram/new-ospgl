#pragma once
#include <string>
#include <utf8/utf8.h>

class UTF8Util
{
public:
	// Should work for counting characters in non-weird stuff (don't use emojis!)
	static int count_codepoints(const std::string& str)
	{
		int count = 0;
		const char* p = &str[0];
		while(*p)
		{
			if((*p & 0xC0) != 0x80)
			{
				count++;
			}
			p++;
		}

		return count;
	}

};