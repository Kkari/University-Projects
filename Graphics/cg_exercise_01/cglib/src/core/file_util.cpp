#include <cglib/core/file_util.h>

#include <fstream>

bool file_exists(const char* filename)
{
	std::ifstream f(filename, std::ios::binary);

	return f.is_open();
}

char* file_read(const char* path)
{
	std::ifstream f(path, std::ios_base::binary);

	if (!f.is_open())
		return 0;

	f.seekg(0, std::ios_base::end);
	unsigned int length = static_cast<unsigned int>(f.tellg());
	f.seekg(0);

	char* buffer = new char[length + 1];
	
	f.read(buffer, length);
	buffer[length] = 0;

	return buffer;
}
