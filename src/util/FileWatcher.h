#pragma once
#include <string>
#include <filesystem>

class FileWatcher
{
private:

	std::string watch_path;
	std::filesystem::file_time_type watch;

public:

	bool has_changed()
	{
		if (watch_path != "")
		{
			std::error_code code;
			auto new_watch = std::filesystem::last_write_time(watch_path, code);
			if (new_watch != watch)
			{
				watch = new_watch;
				return true;
			}

			return false;
		}

		return false;
	}

	FileWatcher(const std::string& path)
	{
		watch_path = path;
		std::error_code code;
		watch = std::filesystem::last_write_time(path, code);
	}

	FileWatcher()
	{
		watch_path = "";
	}
};