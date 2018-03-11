#ifndef UTILITIES_H__
#define UTILITIES_H__

#include <string>

namespace Utilities
{
	const static std::string WHITE_SPACE_CHARS{" \t\r\n"};

	std::string Trim(const std::string& s, const std::string& chars = WHITE_SPACE_CHARS)
	{
		auto pos1 = s.find_first_not_of(WHITE_SPACE_CHARS);
		auto pos2 = s.find_last_not_of(WHITE_SPACE_CHARS);
		if (pos1 == std::string::npos) {
			pos1 = 0;
		}
		if (pos2 == std::string::npos || pos2 < pos1) {
			pos2 = pos1 - 1;
		}
		return s.substr(pos1, pos2 - pos1 + 1);
	}
}

#endif
