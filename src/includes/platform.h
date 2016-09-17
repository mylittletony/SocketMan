#ifdef _WIN64
   //define something for Windows (64-bit)
#elif defined _WIN32
   //define something for Windows (32-bit)
#elif defined __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
        // define something for simulator
    #elif TARGET_OS_IPHONE
        // define something for iphone
    #else
        #define TARGET_OS_OSX 1
        // define something for OSX
    #endif

#elif defined __OPENWRT__
  #define OS "OPENWRT"
  #define NETWORK_ORIGINAL "/etc/config/network.orig"
  #define NETWORK_FILE "/etc/config/network"
  #define NETWORK_BAK "/etc/config/network.backup"
  #define DHCP_LEASES "/tmp/dhcp.leases"

#elif defined __linux
  #define OS "LINUX"
  #define NETWORK_ORIGINAL "/tmp/network.orig"
  #define NETWORK_FILE "/tmp/network"
  #define NETWORK_BAK "/tmp/network.backup"
  #define DHCP_LEASES "/tmp/dhcp.leases"

#elif defined __unix // all unices not caught above
    // Unix
#elif defined __posix
    // POSIX
#endif

