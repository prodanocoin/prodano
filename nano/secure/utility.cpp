#include <nano/lib/config.hpp>
#include <nano/secure/utility.hpp>
#include <nano/secure/working.hpp>

#include <random>

static std::vector<std::filesystem::path> all_unique_paths;

std::filesystem::path nano::working_path (nano::networks network)
{
	auto result (nano::app_path ());
	switch (network)
	{
		case nano::networks::invalid:
			release_assert (false);
			break;
		case nano::networks::banano_dev_network:
			result /= "BananoDev";
			break;
		case nano::networks::banano_beta_network:
			result /= "BananoBeta";
			break;
		case nano::networks::banano_live_network:
			result /= "BananoData";
			break;
		case nano::networks::banano_test_network:
			result /= "BananoTest";
			break;
	}
	return result;
}

std::filesystem::path nano::unique_path (nano::networks network)
{
	std::random_device rd;
	std::mt19937 gen (rd ());
	std::uniform_int_distribution<> dis (0, 15);

	const char * hex_chars = "0123456789ABCDEF";
	std::string random_string;
	random_string.reserve (32);

	for (int i = 0; i < 32; ++i)
	{
		random_string += hex_chars[dis (gen)];
	}

	auto result = working_path (network) / random_string;
	all_unique_paths.push_back (result);
	return result;
}

void nano::remove_temporary_directories ()
{
	for (auto & path : all_unique_paths)
	{
		boost::system::error_code ec;
		std::filesystem::remove_all (path, ec);
		if (ec)
		{
			std::cerr << "Could not remove temporary directory: " << ec.message () << std::endl;
		}

		// lmdb creates a -lock suffixed file for its MDB_NOSUBDIR databases
		auto lockfile = path;
		lockfile += "-lock";
		std::filesystem::remove (lockfile, ec);
		if (ec)
		{
			std::cerr << "Could not remove temporary lock file: " << ec.message () << std::endl;
		}
	}
}

namespace nano
{
/** A wrapper for handling signals */
std::function<void ()> signal_handler_impl;
void signal_handler (int sig)
{
	if (signal_handler_impl != nullptr)
	{
		signal_handler_impl ();
	}
}
}
