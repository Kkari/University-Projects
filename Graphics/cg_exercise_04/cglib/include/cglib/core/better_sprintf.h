#pragma once

/**
 * Better sprintf.
 */
template <class... Args>
std::string sprintf(std::string const& format, Args const&... args)
{
#ifndef WIN32
	#define cglib_snprintf std::snprintf
#else
	#define cglib_snprintf _snprintf
#endif

	std::size_t const num_bytes = cglib_snprintf(nullptr, 0, format.c_str(), args...);

	std::vector<char> buffer(num_bytes+1);
	std::size_t const num_bytes_written = cglib_snprintf(&buffer[0], buffer.size(),
		format.c_str(), args...);

	cg_assert(num_bytes == num_bytes_written);
	return std::string(buffer.data());

#undef cglib_snprintf
}
