#include "FileSystem.h"
#include <fstream>
#include <filesystem>
#include "tinyfiledialogs.h"

namespace FileSystem
{
	bool FileExists(const std::string& path)
	{
		return std::filesystem::exists(path);
	}

	std::string ReadFile(const std::string& path)
	{
		if (!FileExists(path))
			return "";

		std::string fileContent;
		std::string currentLine;

		std::ifstream file(path);

		while (getline(file, currentLine))
		{
			fileContent += currentLine + "\n";
		}

		file.close();

		return fileContent;
	}

	std::string SaveImage()
	{
		char const* lFilterPatterns[1] = { "*.png" };
		char* path = tinyfd_saveFileDialog("Save an image", "", 1, lFilterPatterns, "Image file(.png)");

		// Cancel button clicked
		if (path == nullptr)
			return "";

		return std::string(path);
	}
}