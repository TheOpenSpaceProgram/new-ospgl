#include <string>

#include "dep/miniz.h"
#include <rang.hpp>
#include "define.h"

#include "PkgInfo.h"

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

	// updating will overwrite any outdated packages regardless of ow_mode
	static bool fetch(const std::string& url, OverwriteMode ow_mode, bool updating);
};