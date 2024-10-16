#ifndef __WON_MISCUTIL_H__
#define __WON_MISCUTIL_H__
#include "wonshared.h"

#include <string>

namespace WONAPI
{

bool GetBrowserCommandLineFromRegistry(std::string& sBrowser);
bool Browse(const std::string& sURL);

}

#endif
