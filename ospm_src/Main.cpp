#include <argh.h>
#include "Help.h"
#include "Fetch.h"
#include "define.h"
#include <filesystem>

namespace fs = std::filesystem;

std::string res_folder;

int main(int argc, char** argv)
{
	argh::parser args(argc, argv);

	// TODO: Allow other resource folders
	res_folder = "./res";

	// Check that we are running inside of a valid OSP directory
	if (!fs::exists(res_folder))
	{
		COUT_ERROR << "Resource folder not found, make sure you run OSPM inside your OSP install" << std::endl;
		abort();
	}

	if (args[{"-h", "--help"}])
	{
		Help::show_menu();
	}
	else if (args[1] == "fetch")
	{
		Fetch::OverwriteMode ow_mode = Fetch::PROMPT;
		
		std::string prompt_mode_str = "prompt";
		args("overwrite", "prompt") >> prompt_mode_str;
		if (prompt_mode_str == "prompt")
		{
			ow_mode = Fetch::PROMPT;
		}
		else if (prompt_mode_str == "yes")
		{
			ow_mode = Fetch::YES;
		}
		else if (prompt_mode_str == "no")
		{
			ow_mode = Fetch::NO;
		}
		else
		{
			COUT_ERROR << "Invalid overwrite mode provided" << std::endl;
			abort();
		}
		
		bool force = args["force"];

		Fetch::fetch(args[2], ow_mode, force);
	}
}