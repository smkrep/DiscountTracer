#include "lamoda.h"
#include <fstream>


#ifdef _WIN32
extern "C" __declspec(dllexport) bool parse(std::string & filecontents)
{
	std::string oldpricemarker = "old_price_amount";

	std::size_t found = filecontents.find(oldpricemarker);
	if (found != std::string::npos) {
		return true;
	}
	else {
		return false;
	}
}
#else
extern "C" bool parse(std::string & filecontents)
{
	std::string oldpricemarker = "old_price_amount";

	std::size_t found = filecontents.find(oldpricemarker);
	if (found != std::string::npos) {
		return true;
	}
	else {
		return false;
	}
}
#endif
