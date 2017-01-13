## Building for OpenWRT

SocketMan is tested to run on OpenWRT CC and the current trunk.

The Makefile.sample will build the SocketMan repo for OpenWRT.

To build it, add one variables to the Makefile:

PKG_SOURCE_VERSION:=commit

Where commit is the latest commit from the SocketMan repository.

**It's essential to set the __OPENWRT__ flag otherwise SocketMan will fail to collect some data for OpenWRT buids**

### Building

After setting up OpenWRT, you would build the package as so:

```
make package/feeds/cucumber/socketman/clean &&\
make package/feeds/cucumber/socketman/compile
```

Make sure to add SocketMan via make menuconfig.
