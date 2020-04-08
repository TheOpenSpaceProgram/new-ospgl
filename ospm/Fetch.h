#include <string>

#include "dep/miniz.h"
#include <rang.hpp>
#include "define.h"

class Fetch
{
public:

	enum OverwriteMode
	{
		PROMPT,
		YES,
		NO
	};

	static std::string download(const std::string& url);

	static bool fetch(const std::string& url, OverwriteMode ow_mode, bool force);
};