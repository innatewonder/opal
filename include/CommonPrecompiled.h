//Common Include and Macros
#include "SuperCommon.h"

#define EMPTY_STRING ""

#include "string.h"
#include <string>
typedef std::string String;

#include "ArgParser.h"
#include "Timer.h"

#define SLEEP_MILLI(x) std::this_thread::sleep_for(std::chrono::milliseconds(x))

#include "Message.h"
#include "HBThread.h"

typedef std::vector<std::string> StringVec;
typedef StringVec::iterator      StringVecIt;
#include "FolderHandle.h"
#include "FileHandle.h"
#include "FilesystemReader.h"

//Networking Includes
#if PLATFORM == PLAT_WINDOWS
#define WIN32_LEAN_AND_MEAN
  #include <winsock2.h>
  #include <Ws2tcpip.h>
  #undef min
  #undef max
  #undef far
  #undef near
  #undef SetPort
#elif PLATFORM == PLAT_MAC
#elif PLATFORM == PLAT_UNIX
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <fcntl.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif
#include <stdlib.h>

typedef void GraphicsWindow;

#include "Memory.h"

typedef u32 UID;
#include "CoreUtils.h"
#include "Factory.h"

#include "Component.h"
#include "System.h"

#include "MessageSystem.h"
#include "Core.h"
