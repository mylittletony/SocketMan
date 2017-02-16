## Building for OpenWRT

SocketMan is tested to run on OpenWRT CC and the current trunk.

The Makefile.sample will build the SocketMan repo for OpenWRT.

To build it, add one variables to the Makefile:

PKG_SOURCE_VERSION:=commit

Where commit is the latest commit from the SocketMan repository.

**It's essential to set the __OPENWRT__ flag otherwise SocketMan will fail to collect some data for OpenWRT builds.**

### Building

After setting up OpenWRT, you would build the package as so:

```
make package/feeds/cucumber/socketman/clean &&\
make package/feeds/cucumber/socketman/compile
```

Make sure to add SocketMan via make menuconfig.

### Installing

Send your binary / ipk to your device:

```
scp bin/ar71xx/packages/cucumber/socketman_v1_ar71xx.ipk root@$IP:/tmp/
```

Where IP == your device IP.

Install the package with:

```
opkg install socketman_v1_ar71xx.ipk
```

### Common errors

After running the install, you see this:

```
Installing socketman (v1) to root...
Collected errors:
 * satisfy_dependencies_for: Cannot satisfy the following dependencies for socketman:
 *      curl *  libcurl *       libopenssl *    libmosquitto *
 * opkg_install_cmd: Cannot install package socketman.
 ```

 You're missing some dependencies!

 ```
 opkg update && \
 opkg install curl libopenssl
 ```

 Then try to install things again.
 
 ### socketman.init
 
Here you will find a working example of the socketman init.
 
Add this init in a ```files``` directory in the same place as the ```Makefile``` in your package dir.
Procd is enabled and a respaw threshold is added. 
The init will also populate ```/etc/mac``` with the WAN MAC which is required by SocketMan.

### Dependencies

 - curl
 - json-c
 - mosquitto
 - lnl-3
 - lnl-route-3
 - lnl-genl-3
 - lz
