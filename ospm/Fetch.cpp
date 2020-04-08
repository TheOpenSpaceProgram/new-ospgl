#include "Fetch.h"
#include "dep/httplib.h"
#include <sstream>
#include <cpptoml.h>
#include <filesystem>

namespace fs = std::filesystem;

std::string Fetch::download(const std::string & url)
{
	// Sanitize URL, cut off the http(s):// part if present
	std::string sane_url = url;
	bool secure = true;

	if (sane_url.substr(0, 7) == "http://")
	{
		sane_url = sane_url.substr(7);
		secure = false;
	}
	else if (sane_url.substr(0, 8) == "https://")
	{
		sane_url = sane_url.substr(8);
		secure = true;
	}

	std::string host = sane_url.substr(0, sane_url.find_first_of('/'));
	std::string path = sane_url.substr(sane_url.find_first_of('/'));

	std::string secure_str = "HTTP";
	if (secure)
	{
		secure_str = "HTTPS";
	}

	std::cout << rang::fgB::blue << "Fetching (" << secure_str << ") " << host << path << rang::fg::reset << std::endl;

	httplib::Client* client;

	if (secure)
	{
		httplib::SSLClient* sclient = new httplib::SSLClient(host);
		sclient->enable_server_certificate_verification(false);
		client = sclient;
	}
	else
	{
		client = new httplib::Client(host);
	}

	client->set_follow_location(true);

	int put_chars = 0;

	auto res = client->Get(path.c_str(), [&put_chars](uint64_t len, uint64_t total)
	{
		// Remove old line
		while (put_chars > 0)
		{
			std::cout << "\b";
			put_chars--;
		}

		int percent = len * 100 / total;

		std::stringstream ss;
		ss << len << "/" << total << " bytes (" << percent << "%)";
		std::string ss_str = ss.str();

		put_chars = ss_str.size();

		std::cout << ss_str;

		// False stops the download
		return true;
	});

	std::cout << std::endl;

	if (res)
	{
		std::cout << rang::fgB::green << "Download successful" << rang::fg::reset << std::endl;
		return res->body;
	}
	else
	{
		COUT_ERROR << "Could not obtain a reply" << std::endl;
	}

	return "";
}

bool Fetch::fetch(const std::string& url, OverwriteMode ow_mode, bool updating)
{
	std::string data = download(url);


	// Check the file, it must be a .zip archive
	if (!(data[0] == 0x50 && data[1] == 0x4b && data[2] == 0x03 && data[3] == 0x04))
	{
		COUT_ERROR << "Downloaded file was not a ZIP file, signature not found" << std::endl;
		return false;
	}
	else
	{
		size_t datalen = data.size();
		mz_zip_archive zip_archive;
		memset(&zip_archive, 0, sizeof(zip_archive));

		if (!mz_zip_reader_init_mem(&zip_archive, data.data(), data.size(), 0))
		{
			COUT_ERROR << "Miniz could not understand the ZIP file, it may be corrupted" << std::endl;
			return false;
		}

		// Find metadata file
		bool found_metadata = false;
		char* metadata_file;

		// Find metadata file, it must be directly under the root directory and named
		// package.toml
		for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++)
		{
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
			{
				COUT_ERROR << "Miniz could not understand the ZIP file, it may be corrupted" << std::endl;
				mz_zip_reader_end(&zip_archive);
				return false;
			}

			std::string fname = std::string(file_stat.m_filename);
			
			// Check that it's the root zip directory
			if (fname == "package.toml")
			{
				found_metadata = true;
				
				size_t buffer_size = file_stat.m_uncomp_size + 1;
				metadata_file = (char*)malloc(buffer_size * sizeof(char));
				mz_zip_reader_extract_file_to_mem(&zip_archive, file_stat.m_filename, metadata_file, buffer_size, 0);
				metadata_file[buffer_size - 1] = 0;

				break;
			}
		}

		if (!found_metadata)
		{
			COUT_ERROR << "Could not find package.toml, package is incorrect or corrupted" << std::endl;
			return false;
		}

		using table_ptr = std::shared_ptr<cpptoml::table>;

		// Parse the metadata file (what we need)
		try
		{
			PkgInfo pkg; pkg.load_from_memory(metadata_file);

	
			std::string final_path = res_folder + "/" + pkg.folder + "/";
			
			// Is a package already there?
			if (fs::exists(final_path))
			{
				if (fs::exists(final_path + "package.toml"))
				{
					PkgInfo opkg; opkg.load_from_file(final_path + "package.toml");

					bool overwrite = false;

					// Check versions
					if (pkg.version_num > opkg.version_num && updating &&
						pkg.id == opkg.id)
					{
						// We only update on bigger new version and same package id

						std::cout << "Updating package" << std::endl;
						overwrite = true;
					}
					else
					{
						std::cout << "A package already exists at " + final_path << std::endl;
					
						std::cout << "Installing (" << pkg.id << ")" << " version: " << 
							pkg.version << " (" << pkg.version_num << ")" << std::endl;

						std::cout << "On disk    (" << opkg.id << ")" << " version: " <<
							opkg.version << " (" << opkg.version_num << ")" << std::endl;

						if (ow_mode == YES)
						{
							overwrite = true;
						}
						else if (ow_mode == NO)
						{
							overwrite = false;
						}
						else
						{
							std::cout << rang::fgB::yellow << "Would you like to overwrite it? ";
							overwrite = yn_prompt();

							if (overwrite)
							{
								std::cout << rang::fg::reset << "Overwriting" << std::endl;
							}
							else
							{
								std::cout << rang::fg::reset << "Ignoring package" << std::endl;
							}
						}
					}

					if (overwrite)
					{
						// Delete the old folder
						fs::remove_all(final_path);
					}
					else
					{
						return true;
					}
				}
				else
				{
					// We don't know what to do in this case, so error
					COUT_ERROR << "Folder " << final_path << " exists but is not a package!" << std::endl;
					return false;
				}
			}

			// If we reach here, we must write the data
			int written_files = 0;
			int written_bytes = 0;
			for (int i = 0; i < (int)mz_zip_reader_get_num_files(&zip_archive); i++)
			{
				mz_zip_archive_file_stat file_stat;
				if (!mz_zip_reader_file_stat(&zip_archive, i, &file_stat))
				{
					COUT_ERROR << "Miniz could not understand the ZIP file, it may be corrupted" << std::endl;
					mz_zip_reader_end(&zip_archive);
					return false;
				}

				if (!file_stat.m_is_directory)
				{
					std::string sub_path = file_stat.m_filename;
					std::string full_path = res_folder + "/" + pkg.folder + "/" + sub_path;

					//std::cout << "Writing file " << sub_path << " to " << full_path << std::endl;

					char* file_buffer = (char*)malloc(file_stat.m_uncomp_size);
					mz_zip_reader_extract_file_to_mem(&zip_archive, file_stat.m_filename, file_buffer, file_stat.m_uncomp_size, 0);

					// Cut out the last piece of the path so that we don't create a folder named as the file
					std::string dir_path = full_path.substr(0, full_path.find_last_of('/'));
					fs::create_directories(dir_path);

					std::fstream out(full_path, std::ios::out | std::ios::binary);
					if (out.bad())
					{
						COUT_ERROR << "Could not write file " << full_path << std::endl;
						free(file_buffer);
						return false;
					}

					out.write(file_buffer, file_stat.m_uncomp_size);
					out.close();
					free(file_buffer);

					written_files++;
					written_bytes += file_stat.m_uncomp_size;
				}
			}

			std::cout << "Written " << written_files << " files taking " << written_bytes << " bytes" << std::endl;

		}
		catch (cpptoml::parse_exception excp)
		{
			COUT_ERROR << "Metadata file could not be parsed. Error: " << excp.what() << std::endl;
			return false;
		}
		catch (...)
		{
			COUT_ERROR << "Unknown error parsing metadata file" << std::endl;
			return false;
		}


		// Close it
		mz_zip_reader_end(&zip_archive);

		return true;
	}
}