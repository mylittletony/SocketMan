## Full OpenWRT build instructions

The following guide explains how to completely build the package from scratch. It walks you through building OpenWRT and finally compiling the package.

This process describes the build process on an Ubuntu 16.04 system. The commands used will be the same on other systems, but the outputs observed may vary from system to system.

To build OpenWRT, including the socketman module, the following commands need to be executed in the order shown below from a terminal window. Once built, the socketman module may be transferred as a standalone file to the target OpenWRT device.

A summary of the commands used to build OpenWRT & socketman is shown below. This is followed by the detailed output from each stage later in this document. 

Many of the steps presented take only a few minutes to complete, but the final 'make' command make take several hours to complete, depending on the resources of your build system.

Here are the build process commands (these executed from terminal - do not enter lines beginning with a '#', these are comments). The commands were all executed as a standard (non-root) user:
```
sudo apt-get update
sudo apt-get install git-core build-essential libssl-dev libncurses5-dev unzip gawk zlib1g-dev libnl-3-dev

git clone https://github.com/openwrt/openwrt.git

cd openwrt
echo "src-git cucumber http://github.com/cucumber-tony/cucumber-feeds.git;for-15.05" >> feeds.conf.default

./scripts/feeds update -a
./scripts/feeds install -a

make menuconfig
# Enter the following options in menu system presented by menuconfig:
#
# -> Network -> Socketman [M]
# -> Image Configuration [*]) -> Separate feed repositories [*] -> Enable feed cucumber [*]
# <Save> (writes to .config)
# <Exit>
# <Exit>
  
# Note the command below may take several hours to complete
make -j`grep -c processor /proc/cpuinfo`
  
cd bin/ar71xx/packages/cucumber
ls -l
  
# you should now see the package 'socketman_???????_ar71xx.ipk' listed - you're done! ('?' will be shown as digits in your build)
```
### Full Build Output

For reference purposes, the commands and full output of each stage of the build process is shown below:

```
nbowden@nbowden-Aspire-E1-532:~/dev$ git clone https://github.com/openwrt/openwrt.git
Cloning into 'openwrt'...
remote: Counting objects: 360799, done.
remote: Compressing objects: 100% (13/13), done.
remote: Total 360799 (delta 11), reused 9 (delta 9), pack-reused 360777
Receiving objects: 100% (360799/360799), 132.89 MiB | 3.43 MiB/s, done.
Resolving deltas: 100% (241421/241421), done.
Checking connectivity... done.
Checking out files: 100% (7357/7357), done.
nbowden@nbowden-Aspire-E1-532:~/dev$ 
```

```
nbowden@nbowden-Aspire-E1-532:~/dev$ cd openwrt
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ ls -l
total 84
-rw-rw-r--  1 nbowden nbowden   179 Feb  6 05:08 BSDmakefile
drwxrwxr-x  2 nbowden nbowden  4096 Feb  6 05:08 config
-rw-rw-r--  1 nbowden nbowden   576 Feb  6 05:08 Config.in
drwxrwxr-x  2 nbowden nbowden  4096 Feb  6 05:08 docs
-rw-rw-r--  1 nbowden nbowden   457 Feb  6 05:08 feeds.conf.default
drwxrwxr-x  3 nbowden nbowden  4096 Feb  6 05:08 include
-rw-rw-r--  1 nbowden nbowden 17992 Feb  6 05:08 LICENSE
-rw-rw-r--  1 nbowden nbowden  2670 Feb  6 05:08 Makefile
drwxrwxr-x 11 nbowden nbowden  4096 Feb  6 05:08 package
-rw-rw-r--  1 nbowden nbowden  1272 Feb  6 05:08 README
-rw-rw-r--  1 nbowden nbowden 12237 Feb  6 05:08 rules.mk
drwxrwxr-x  4 nbowden nbowden  4096 Feb  6 05:08 scripts
drwxrwxr-x  6 nbowden nbowden  4096 Feb  6 05:08 target
drwxrwxr-x 12 nbowden nbowden  4096 Feb  6 05:08 toolchain
drwxrwxr-x 57 nbowden nbowden  4096 Feb  6 05:08 tools
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ 
```

```
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ echo "src-git cucumber http://github.com/cucumber-tony/cucumber-feeds.git;for-15.05" >> feeds.conf.default
```

```
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ ./scripts/feeds update -a
Updating feed 'packages' from 'https://github.com/openwrt/packages.git' ...
Cloning into './feeds/packages'...
remote: Counting objects: 3805, done.
remote: Compressing objects: 100% (3181/3181), done.
remote: Total 3805 (delta 163), reused 3089 (delta 108), pack-reused 0
Receiving objects: 100% (3805/3805), 2.31 MiB | 2.84 MiB/s, done.
Resolving deltas: 100% (163/163), done.
Checking connectivity... done.
Create index file './feeds/packages.index' 
Collecting package info: done
Collecting target info: done
Updating feed 'luci' from 'https://github.com/openwrt/luci.git' ...
Cloning into './feeds/luci'...
remote: Counting objects: 4358, done.
remote: Compressing objects: 100% (2376/2376), done.
remote: Total 4358 (delta 984), reused 3019 (delta 473), pack-reused 0
Receiving objects: 100% (4358/4358), 3.72 MiB | 961.00 KiB/s, done.
Resolving deltas: 100% (984/984), done.
Checking connectivity... done.
Create index file './feeds/luci.index' 
Collecting package info: done
Collecting target info: done
Updating feed 'routing' from 'https://github.com/openwrt-routing/packages.git' ...
Cloning into './feeds/routing'...
remote: Counting objects: 367, done.
remote: Compressing objects: 100% (287/287), done.
remote: Total 367 (delta 24), reused 261 (delta 13), pack-reused 0
Receiving objects: 100% (367/367), 233.64 KiB | 0 bytes/s, done.
Resolving deltas: 100% (24/24), done.
Checking connectivity... done.
Create index file './feeds/routing.index' 
Collecting package info: done
Collecting target info: done
Updating feed 'telephony' from 'https://github.com/openwrt/telephony.git' ...
Cloning into './feeds/telephony'...
remote: Counting objects: 178, done.
remote: Compressing objects: 100% (160/160), done.
remote: Total 178 (delta 11), reused 109 (delta 5), pack-reused 0
Receiving objects: 100% (178/178), 109.90 KiB | 0 bytes/s, done.
Resolving deltas: 100% (11/11), done.
Checking connectivity... done.
Create index file './feeds/telephony.index' 
Collecting package info: done
Collecting target info: done
Updating feed 'management' from 'https://github.com/openwrt-management/packages.git' ...
Cloning into './feeds/management'...
remote: Counting objects: 52, done.
remote: Compressing objects: 100% (41/41), done.
remote: Total 52 (delta 9), reused 30 (delta 0), pack-reused 0
Unpacking objects: 100% (52/52), done.
Checking connectivity... done.
Create index file './feeds/management.index' 
Collecting package info: done
Collecting target info: done
Updating feed 'targets' from 'https://github.com/openwrt/targets.git' ...
Cloning into './feeds/targets'...
remote: Counting objects: 159, done.
remote: Compressing objects: 100% (107/107), done.
remote: Total 159 (delta 28), reused 141 (delta 25), pack-reused 0
Receiving objects: 100% (159/159), 149.24 KiB | 0 bytes/s, done.
Resolving deltas: 100% (28/28), done.
Checking connectivity... done.
Create index file './feeds/targets.index' 
Collecting package info: done
Collecting target info: done
Updating feed 'cucumber' from 'http://github.com/cucumber-tony/cucumber-feeds.git;for-15.05' ...
Cloning into './feeds/cucumber'...
remote: Counting objects: 6, done.
remote: Compressing objects: 100% (4/4), done.
remote: Total 6 (delta 0), reused 6 (delta 0), pack-reused 0
Unpacking objects: 100% (6/6), done.
Checking connectivity... done.
Create index file './feeds/cucumber.index' 
Collecting package info: done
Collecting target info: done
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$
```

```
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ ./scripts/feeds install -a
Installing all packages from feed packages.
Installing package 'acl' from packages
Installing package 'attr' from packages
Installing package 'acme' from packages
Installing package 'netcat' from packages
Installing package 'luci-base' from luci
Installing package 'luci-lib-nixio' from luci
Installing package 'luci-lib-ip' from luci
Installing package 'luci-lib-jsonc' from luci
Installing package 'luci-app-uhttpd' from luci
Installing package 'acpid' from packages
Installing package 'adblock' from packages
Installing package 'addrwatch' from packages
Installing package 'aggregate' from packages
Installing package 'aiccu' from packages
Installing package 'gnutls' from packages
Installing package 'libtasn1' from packages
Installing package 'p11-kit' from packages
Installing package 'cryptodev-linux' from packages
Installing package 'aircrack-ng' from packages
Installing package 'ethtool' from packages
Installing package 'procps-ng' from packages
Installing package 'pciutils' from packages
Installing package 'kmod' from packages
Installing package 'alpine' from packages
Installing package 'libpam' from packages
Installing package 'alsa-lib' from packages
Installing package 'alsa-utils' from packages
Installing package 'announce' from packages
Installing package 'ap51-flash' from packages
Installing package 'apache' from packages
Installing package 'apr' from packages
Installing package 'apr-util' from packages
Installing package 'expat' from packages
Installing package 'sqlite3' from packages
Installing package 'pcre' from packages
Installing package 'unixodbc' from packages
Installing package 'postgresql' from packages
Installing package 'apcupsd' from packages
Installing package 'libgd' from packages
Installing package 'libjpeg' from packages
Installing package 'libpng' from packages
Installing package 'apinger' from packages
Installing package 'aria2' from packages
Installing package 'libssh2' from packages
Installing package 'libxml2' from packages
Installing package 'arp-scan' from packages
Installing package 'at' from packages
Installing package 'atftp' from packages
Installing package 'autoconf' from packages
Installing package 'm4' from packages
Installing package 'perl' from packages
Installing package 'db47' from packages
Installing package 'gdbm' from packages
Installing package 'automake' from packages
Installing package 'autossh' from packages
Installing package 'avahi' from packages
Installing package 'libdaemon' from packages
Installing package 'intltool' from packages
Installing package 'dbus' from packages
Installing package 'avrdude' from packages
Installing package 'libftdi1' from packages
Installing package 'confuse' from packages
Installing package 'avro' from packages
Installing package 'jansson' from packages
Installing package 'xz' from packages
Installing package 'bash' from packages
Installing package 'bandwidthd' from packages
Installing package 'php7' from packages
Installing package 'icu' from packages
Installing package 'openldap' from packages
Installing package 'cyrus-sasl' from packages
Installing package 'libmcrypt' from packages
Installing package 'mysql' from packages
Installing package 'banhosts' from packages
Installing package 'bc' from packages
Installing package 'bcp38' from packages
Installing package 'beep' from packages
Installing package 'bind' from packages
Installing package 'bitlbee' from packages
Installing package 'glib2' from packages
Installing package 'libffi' from packages
Installing package 'bluelog' from packages
Installing package 'bluez' from packages
Installing package 'python' from packages
Installing package 'libical' from packages
Installing package 'bmon' from packages
Installing package 'bogofilter' from packages
Installing package 'bonnie++' from packages
Installing package 'boost' from packages
Installing package 'python3' from packages
Installing package 'bridge-utils' from packages
Installing package 'libarchive' from packages
Installing package 'btrfs-progs' from packages
Installing package 'bwm-ng' from packages
Installing package 'canutils' from packages
Installing package 'ccid' from packages
Installing package 'pcsc-lite' from packages
Installing package 'ccrypt' from packages
Installing package 'gptfdisk' from packages
Installing package 'cgi-io' from packages
Installing package 'chaosvpn' from packages
Installing package 'tinc' from packages
Installing package 'chardet' from packages
Installing package 'check' from packages
Installing package 'chrony' from packages
Installing package 'libcap' from packages
Installing package 'pps-tools' from packages
Installing package 'cifs-utils' from packages
Installing package 'clamav' from packages
Installing package 'classpath' from packages
Installing package 'file' from packages
Installing package 'cmdpad' from packages
Installing package 'collectd' from packages
Installing package 'libmodbus' from packages
Installing package 'libgcrypt' from packages
Installing package 'libgpg-error' from packages
Installing package 'nut' from packages
Installing package 'net-snmp' from packages
Installing package 'tcp_wrappers' from packages
Installing package 'owfs' from packages
Installing package 'liboping' from packages
Installing package 'rrdtool1' from packages
Installing package 'lm-sensors' from packages
Installing package 'rtklib' from packages
Installing package 'coova-chilli' from packages
WARNING: No feed for package 'libmatrixssl' found, maybe it's already part of the standard packages?
Installing package 'coreutils' from packages
Installing package 'crelay' from packages
Installing package 'hidapi' from packages
Installing package 'crtmpserver' from packages
Installing package 'cryptsetup' from packages
Installing package 'lvm2' from packages
Installing package 'cshark' from packages
Installing package 'luci' from luci
Installing package 'luci-mod-admin-full' from luci
Installing package 'luci-theme-bootstrap' from luci
Installing package 'luci-app-firewall' from luci
Installing package 'luci-proto-ppp' from luci
Installing package 'luci-proto-ipv6' from luci
Installing package 'daemonlogger' from packages
Installing package 'libdnet' from packages
Installing package 'dansguardian' from packages
Installing package 'darkstat' from packages
Installing package 'davfs2' from packages
Installing package 'neon' from packages
Installing package 'ddns-scripts' from packages
Installing package 'debootstrap' from packages
Installing package 'dfu-programmer' from packages
Installing package 'dhcp-forwarder' from packages
Installing package 'dhcpcd' from packages
Installing package 'diffutils' from packages
Installing package 'django' from packages
Installing package 'django-appconf' from packages
Installing package 'django-compressor' from packages
Installing package 'django-constance' from packages
Installing package 'django-jsonfield' from packages
Installing package 'django-picklefield' from packages
Installing package 'django-postoffice' from packages
Installing package 'django-restframework' from packages
Installing package 'django-statici18n' from packages
Installing package 'dkjson' from packages
Installing package 'dmapd' from packages
Installing package 'libdmapsharing' from packages
Installing package 'libsoup' from packages
Installing package 'mdnsresponder' from packages
Installing package 'gstreamer1' from packages
Installing package 'gst1-plugins-base' from packages
Installing package 'libogg' from packages
Installing package 'opus' from packages
Installing package 'libtheora' from packages
Installing package 'libvorbis' from packages
Installing package 'liboil' from packages
Installing package 'vips' from packages
Installing package 'libexif' from packages
Installing package 'dmidecode' from packages
Installing package 'dnscrypt-proxy' from packages
Installing package 'libsodium' from packages
Installing package 'dosfstools' from packages
Installing package 'dovecot' from packages
Installing package 'ldns' from packages
Installing package 'dtndht' from packages
Installing package 'dump1090' from packages
Installing package 'rtl-sdr' from packages
Installing package 'dvtm' from packages
Installing package 'dynapoint' from packages
Installing package 'e2guardian' from packages
Installing package 'ecdsautils' from packages
Installing package 'libuecc' from packages
Installing package 'elektra' from packages
Installing package 'swig' from packages
Installing package 'yajl' from packages
Installing package 'emailrelay' from packages
Installing package 'erlang' from packages
Installing package 'esniper' from packages
Installing package 'espeak' from packages
Installing package 'portaudio' from packages
Installing package 'et_xmlfile' from packages
Installing package 'etherwake' from packages
Installing package 'eudev' from packages
Installing package 'gperf' from packages
Installing package 'evtest' from packages
Installing package 'libextractor' from packages
Installing package 'flac' from packages
Installing package 'giflib' from packages
Installing package 'libmpeg2' from packages
Installing package 'ffmpeg' from packages
Installing package 'libx264' from packages
Installing package 'lame' from packages
Installing package 'tiff' from packages
Installing package 'f2fs-tools' from packages
Installing package 'faad2' from packages
Installing package 'fakeidentd' from packages
Installing package 'fastd' from packages
Installing package 'nacl' from packages
Installing package 'fcgi' from packages
Installing package 'fdk-aac' from packages
Installing package 'fdm' from packages
Installing package 'tdb' from packages
Installing package 'fftw3' from packages
Installing package 'findutils' from packages
Installing package 'flashrom' from packages
Installing package 'flent-tools' from packages
Installing package 'flup' from packages
Installing package 'fontconfig' from packages
Installing package 'freetype' from packages
Installing package 'forked-daapd' from packages
Installing package 'libunistring' from packages
Installing package 'libantlr3c' from packages
Installing package 'mxml' from packages
Installing package 'libplist' from packages
Installing package 'protobuf-c' from packages
Installing package 'fossil' from packages
Installing package 'fping' from packages
Installing package 'freeradius2' from packages
Installing package 'freeradius3' from packages
Installing package 'libtalloc' from packages
Installing package 'fswebcam' from packages
Installing package 'fwknop' from packages
Installing package 'gnupg' from packages
Installing package 'gammu' from packages
Installing package 'gcc' from packages
Installing package 'git' from packages
Installing package 'perl-cgi' from packages
Installing package 'perl-html-parser' from packages
Installing package 'perl-html-tagset' from packages
Installing package 'gitolite' from packages
Installing package 'openssh' from packages
Installing package 'gkermit' from packages
Installing package 'glpk' from packages
Installing package 'gnunet' from packages
Installing package 'libidn' from packages
Installing package 'pulseaudio' from packages
Installing package 'speex' from packages
Installing package 'libsndfile' from packages
Installing package 'gnurl' from packages
Installing package 'libmicrohttpd' from packages
Installing package 'libugpio' from packages
Installing package 'gpsd' from packages
Installing package 'grep' from packages
Installing package 'grilo' from packages
Installing package 'grilo-plugins' from packages
Installing package 'gst1-libav' from packages
Installing package 'gst1-plugins-bad' from packages
Installing package 'gst1-plugins-good' from packages
Installing package 'libv4l' from packages
Installing package 'libvpx' from packages
Installing package 'gst1-plugins-ugly' from packages
Installing package 'libid3tag' from packages
Installing package 'libmad' from packages
Installing package 'gunicorn' from packages
Installing package 'gzip' from packages
Installing package 'haproxy' from packages
Installing package 'hamlib' from packages
Installing package 'haserl' from packages
Installing package 'haveged' from packages
Installing package 'hd-idle' from packages
Installing package 'hdparm' from packages
Installing package 'hfsprogs' from packages
Installing package 'horst' from packages
Installing package 'htop' from packages
Installing package 'htpdate' from packages
Installing package 'https-dns-proxy' from packages
Installing package 'c-ares' from packages
Installing package 'libev' from packages
Installing package 'hub-ctrl' from packages
Installing package 'i2c-tools' from packages
Installing package 'ibrcommon' from packages
Installing package 'ibrdtn' from packages
Installing package 'ibrdtn-tools' from packages
Installing package 'ibrdtnd' from packages
Installing package 'icecast' from packages
Installing package 'libxslt' from packages
Installing package 'libvorbisidec' from packages
Installing package 'ices' from packages
Installing package 'libshout' from packages
Installing package 'ifstat' from packages
Installing package 'inadyn' from packages
Installing package 'io' from packages
Installing package 'iodine' from packages
Installing package 'iotivity' from packages
Installing package 'ipsec-tools' from packages
Installing package 'iptraf-ng' from packages
Installing package 'irssi' from packages
Installing package 'isc-dhcp' from packages
Installing package 'jamvm' from packages
Installing package 'jdcal' from packages
Installing package 'joe' from packages
Installing package 'jool' from packages
Installing package 'json4lua' from packages
Installing package 'luasocket' from packages
Installing package 'keepalived' from packages
Installing package 'kismet' from packages
Installing package 'klish' from packages
Installing package 'exfat-nofuse' from packages
Installing package 'lttng-modules' from packages
Installing package 'mtd-rw' from packages
Installing package 'openvswitch' from packages
Installing package 'siit' from packages
Installing package 'dmx_usb_module' from packages
Installing package 'xr_usb_serial_common' from packages
Installing package 'wireguard' from packages
Installing package 'knot' from packages
Installing package 'liburcu' from packages
Installing package 'libedit' from packages
Installing package 'knxd' from packages
Installing package 'kplex' from packages
Installing package 'krb5' from packages
Installing package 'l7-protocols' from packages
Installing package 'lcd4linux' from packages
Installing package 'libmpdclient' from packages
WARNING: No feed for package 'libnmeap' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libnmeap' found, maybe it's already part of the standard packages?
Installing package 'lcdgrilo' from packages
Installing package 'libgee' from packages
Installing package 'vala' from packages
Installing package 'lcdringer' from packages
Installing package 'loudmouth' from packages
Installing package 'less' from packages
Installing package 'lftp' from packages
Installing package 'libaio' from packages
Installing package 'libao' from packages
Installing package 'libartnet' from packages
Installing package 'libaudiofile' from packages
Installing package 'libavl' from packages
Installing package 'libcanfestival' from packages
Installing package 'libcoap' from packages
Installing package 'libdaq' from packages
Installing package 'libdbi-drivers' from packages
Installing package 'libdbi' from packages
Installing package 'libdouble-conversion' from packages
Installing package 'libdrm' from packages
Installing package 'libesmtp' from packages
Installing package 'libestr' from packages
Installing package 'libevdev' from packages
Installing package 'libevent' from packages
Installing package 'eventlog' from packages
Installing package 'libevhtp' from packages
Installing package 'libfastjson' from packages
Installing package 'libftdi' from packages
Installing package 'hiredis' from packages
Installing package 'libhttp-parser' from packages
Installing package 'libimobiledevice' from packages
Installing package 'libusbmuxd' from packages
Installing package 'libinput' from packages
Installing package 'mtdev' from packages
Installing package 'liblo' from packages
Installing package 'lxc' from packages
Installing package 'libseccomp' from packages
Installing package 'luafilesystem' from packages
Installing package 'liblz4' from packages
Installing package 'miniupnpc' from packages
Installing package 'libmms' from packages
Installing package 'mosquitto' from packages
Installing package 'libwebsockets' from packages
Installing package 'libuv' from packages
Installing package 'mpg123' from packages
Installing package 'libmraa' from packages
Installing package 'node' from packages
Installing package 'libnatpmp' from packages
Installing package 'libnet-1.2.x' from packages
Installing package 'libnetfilter-acct' from packages
Installing package 'libnopoll' from packages
Installing package 'openobex' from packages
Installing package 'opensc' from packages
Installing package 'libowfat' from packages
Installing package 'libp11' from packages
Installing package 'qrencode' from packages
Installing package 'libradcli' from packages
Installing package 'ruby' from packages
Installing package 'yaml' from packages
Installing package 'libsamplerate' from packages
Installing package 'sane-backends' from packages
Installing package 'lksctp-tools' from packages
Installing package 'libsearpc' from packages
Installing package 'libsigc++' from packages
Installing package 'libsoxr' from packages
Installing package 'stoken' from packages
Installing package 'libstrophe' from packages
Installing package 'libtool-bin' from packages
Installing package 'libtorrent' from packages
Installing package 'unbound' from packages
Installing package 'unrar' from packages
Installing package 'libupm' from packages
Installing package 'libupnp' from packages
Installing package 'libupnpp' from packages
Installing package 'libuvc' from packages
Installing package 'uvcdynctrl' from packages
Installing package 'libxerces-c' from packages
Installing package 'libzdb' from packages
Installing package 'zmq' from packages
Installing package 'lighttpd' from packages
Installing package 'linknx' from packages
Installing package 'pthsem' from packages
Installing package 'linuxptp' from packages
Installing package 'lispmob' from packages
Installing package 'logrotate' from packages
Installing package 'lpc21isp' from packages
Installing package 'lpeg' from packages
Installing package 'lrzsz' from packages
Installing package 'lsof' from packages
Installing package 'lua-lsqlite3' from packages
Installing package 'lttng-tools' from packages
Installing package 'lttng-ust' from packages
Installing package 'lua-bencode' from packages
Installing package 'lua-cjson' from packages
Installing package 'lua-copas' from packages
Installing package 'lua-coxpcall' from packages
Installing package 'lua-lzlib' from packages
Installing package 'lua-md5' from packages
Installing package 'lua-mobdebug' from packages
Installing package 'lua-mosquitto' from packages
Installing package 'lua-openssl' from packages
Installing package 'lua-penlight' from packages
Installing package 'lua-rings' from packages
Installing package 'lua-rs232' from packages
Installing package 'lua-sha2' from packages
Installing package 'lua-wsapi' from packages
Installing package 'lua-xavante' from packages
Installing package 'luabitop' from packages
Installing package 'luaexpat' from packages
Installing package 'luai2c' from packages
Installing package 'luajit' from packages
Installing package 'lualanes' from packages
Installing package 'luaposix' from packages
Installing package 'luarocks' from packages
Installing package 'unzip' from packages
Installing package 'luasec' from packages
Installing package 'luasoap' from packages
Installing package 'luasql' from packages
Installing package 'luci-app-bcp38' from packages
Installing package 'luci-app-clamav' from packages
Installing package 'luci-app-e2guardian' from packages
Installing package 'luci-app-lxc' from packages
Installing package 'rpcd-mod-lxc' from packages
Installing package 'mwan3-luci' from packages
Installing package 'mwan3' from packages
Installing package 'sqm-scripts' from packages
Installing package 'luci-app-squid' from packages
Installing package 'squid' from packages
Installing package 'lzmq' from packages
Installing package 'mac-telnet' from packages
Installing package 'macchanger' from packages
Installing package 'madplay' from packages
Installing package 'mailman' from packages
Installing package 'postfix' from packages
Installing package 'tinycdb' from packages
Installing package 'python-dns' from packages
Installing package 'mailsend' from packages
Installing package 'make' from packages
Installing package 'mbtools' from packages
Installing package 'mc' from packages
Installing package 'memcached' from packages
Installing package 'micropython' from packages
Installing package 'micropython-lib' from packages
Installing package 'mii-tool' from packages
Installing package 'mini_snmpd' from packages
Installing package 'minicom' from packages
Installing package 'minidlna' from packages
Installing package 'mjpg-streamer' from packages
Installing package 'mksh' from packages
Installing package 'mktorrent' from packages
Installing package 'mmc-utils' from packages
Installing package 'mocp' from packages
Installing package 'monit' from packages
Installing package 'motion' from packages
Installing package 'mpack' from packages
Installing package 'mpc' from packages
Installing package 'mpd' from packages
Installing package 'msmtp' from packages
Installing package 'msmtp-scripts' from packages
Installing package 'xinetd' from packages
Installing package 'mt-st' from packages
Installing package 'mtr' from packages
Installing package 'muninlite' from packages
Installing package 'mutt' from packages
Installing package 'nail' from packages
Installing package 'nano' from packages
Installing package 'nbd' from packages
Installing package 'nmap' from packages
Installing package 'ncdu' from packages
Installing package 'ncp' from packages
Installing package 'netatalk' from packages
Installing package 'netdata' from packages
Installing package 'netdiscover' from packages
Installing package 'netperf' from packages
Installing package 'nfs-kernel-server' from packages
Installing package 'portmap' from packages
Installing package 'nginx' from packages
Installing package 'ngircd' from packages
Installing package 'node-arduino-firmata' from packages
Installing package 'node-serialport' from packages
Installing package 'node-cylon' from packages
Installing package 'node-hid' from packages
Installing package 'nsd' from packages
Installing package 'ntfs-3g' from packages
Installing package 'ntpd' from packages
Installing package 'ntpclient' from packages
Installing package 'ntripcaster' from packages
Installing package 'ntripclient' from packages
Installing package 'ntripserver' from packages
Installing package 'oath-toolkit' from packages
Installing package 'obfsproxy' from packages
Installing package 'python-crypto' from packages
Installing package 'python-pyptlib' from packages
Installing package 'python-yaml' from packages
Installing package 'twisted' from packages
Installing package 'zope-interface' from packages
Installing package 'ocserv' from packages
Installing package 'oggfwd' from packages
Installing package 'ola' from packages
Installing package 'protobuf' from packages
Installing package 'sudo' from packages
Installing package 'open-plc-utils' from packages
Installing package 'open2300' from packages
Installing package 'openconnect' from packages
Installing package 'vpnc-scripts' from packages
Installing package 'opencv' from packages
Installing package 'opennhrp' from packages
Installing package 'openocd' from packages
Installing package 'openpyxl' from packages
Installing package 'opentracker' from packages
Installing package 'opus-tools' from packages
Installing package 'p910nd' from packages
Installing package 'patch' from packages
Installing package 'pen' from packages
Installing package 'perl-compress-bzip2' from packages
Installing package 'perl-dbi' from packages
Installing package 'perl-device-serialport' from packages
Installing package 'perl-device-usb' from packages
Installing package 'perl-inline' from packages
Installing package 'perl-inline-c' from packages
Installing package 'perl-parse-recdescent' from packages
Installing package 'perl-file-sharedir-install' from packages
Installing package 'perl-encode-locale' from packages
Installing package 'perl-file-listing' from packages
Installing package 'perl-http-date' from packages
Installing package 'perl-html-form' from packages
Installing package 'perl-http-message' from packages
Installing package 'perl-io-html' from packages
Installing package 'perl-lwp-mediatypes' from packages
Installing package 'perl-uri' from packages
Installing package 'perl-html-tree' from packages
Installing package 'perl-http-cookies' from packages
Installing package 'perl-http-daemon' from packages
Installing package 'perl-http-negotiate' from packages
Installing package 'perl-http-server-simple' from packages
Installing package 'perl-lockfile-simple' from packages
Installing package 'perl-net-http' from packages
Installing package 'perl-net-telnet' from packages
Installing package 'perl-sub-uplevel' from packages
Installing package 'perl-test-harness' from packages
Installing package 'perl-test-warn' from packages
Installing package 'perl-www' from packages
Installing package 'perl-www-robotrules' from packages
Installing package 'perl-www-curl' from packages
Installing package 'perl-www-mechanize' from packages
Installing package 'perl-xml-parser' from packages
Installing package 'php7-pecl-dio' from packages
Installing package 'php7-pecl-http' from packages
Installing package 'php7-pecl-raphf' from packages
Installing package 'php7-pecl-propro' from packages
Installing package 'php7-pecl-libevent' from packages
Installing package 'pianod' from packages
Installing package 'picocom' from packages
Installing package 'pillow' from packages
Installing package 'pingcheck' from packages
Installing package 'pkg-config' from packages
Installing package 'poco' from packages
Installing package 'polipo' from packages
Installing package 'port-mirroring' from packages
Installing package 'pppossh' from packages
Installing package 'pptpd' from packages
Installing package 'privoxy' from packages
Installing package 'progress' from packages
Installing package 'prosody' from packages
Installing package 'pv' from packages
Installing package 'python-attrs' from packages
Installing package 'python-cffi' from packages
Installing package 'python-pycparser' from packages
Installing package 'python-ply' from packages
Installing package 'python-crcmod' from packages
Installing package 'python-cryptography' from packages
Installing package 'python-enum34' from packages
Installing package 'python-idna' from packages
Installing package 'python-ipaddress' from packages
Installing package 'python-pyasn1' from packages
Installing package 'python-six' from packages
Installing package 'python-dateutil' from packages
Installing package 'python-egenix-mx-base' from packages
Installing package 'python-gmpy2' from packages
Installing package 'python-ldap' from packages
Installing package 'python-mysql' from packages
Installing package 'python-packages' from packages
Installing package 'python-parsley' from packages
Installing package 'python-pcapy' from packages
Installing package 'python-psycopg2' from packages
Installing package 'python-pyasn1-modules' from packages
Installing package 'python-pyopenssl' from packages
Installing package 'python-pyserial' from packages
Installing package 'python-service-identity' from packages
Installing package 'python-txsocksx' from packages
Installing package 'python-urllib3' from packages
Installing package 'python3-bottle' from packages
Installing package 'pytz' from packages
Installing package 'qemu' from packages
Installing package 'quassel-irssi' from packages
Installing package 'quasselc' from packages
Installing package 'radicale' from packages
Installing package 'radsecproxy' from packages
Installing package 'rcssmin' from packages
Installing package 'reaver' from packages
Installing package 'redsocks' from packages
Installing package 'relayctl' from packages
Installing package 'remserial' from packages
Installing package 'rng-tools' from packages
Installing package 'rp-pppoe' from packages
Installing package 'rsync' from packages
Installing package 'rsyslog' from packages
Installing package 'rtl-ais' from packages
Installing package 'rtorrent' from packages
Installing package 'xmlrpc-c' from packages
Installing package 'rxtx' from packages
Installing package 'sbc' from packages
Installing package 'scapy' from packages
Installing package 'screen' from packages
Installing package 'seafile-ccnet' from packages
Installing package 'seafile-seahub' from packages
Installing package 'simplejson' from packages
Installing package 'seafile-server' from packages
Installing package 'ser2net' from packages
Installing package 'serialconsole' from packages
Installing package 'shadow' from packages
Installing package 'shadowsocks-client' from packages
Installing package 'shadowsocks-libev' from packages
Installing package 'shairplay' from packages
Installing package 'shairport' from packages
Installing package 'shairport-sync' from packages
Installing package 'shine' from packages
Installing package 'sipgrep' from packages
Installing package 'sispmctl' from packages
Installing package 'slide-switch' from packages
Installing package 'smartmontools' from packages
Installing package 'smartsnmpd' from packages
Installing package 'smstools3' from packages
Installing package 'sngrep' from packages
Installing package 'snort' from packages
Installing package 'socat' from packages
Installing package 'sockread' from packages
Installing package 'softethervpn' from packages
Installing package 'softflowd' from packages
Installing package 'sox' from packages
Installing package 'spawn-fcgi' from packages
Installing package 'spi-tools' from packages
Installing package 'sqm-scripts-extra' from packages
Installing package 'squashfs-tools' from packages
Installing package 'squeezelite' from packages
Installing package 'sshfs' from packages
Installing package 'sshtunnel' from packages
Installing package 'sslh' from packages
Installing package 'ssmtp' from packages
Installing package 'sstp-client' from packages
Installing package 'stm32flash' from packages
Installing package 'stress' from packages
Installing package 'strongswan' from packages
Installing package 'stunnel' from packages
Installing package 'subversion' from packages
Installing package 'sumo' from packages
Installing package 'svox' from packages
Installing package 'syslog-ng' from packages
Installing package 'sysstat' from packages
Installing package 'tar' from packages
Installing package 'taskwarrior' from packages
Installing package 'tayga' from packages
Installing package 'tcl' from packages
Installing package 'tcpreplay' from packages
Installing package 'tcpproxy' from packages
Installing package 'tcsh' from packages
Installing package 'tgt' from packages
Installing package 'tinyproxy' from packages
Installing package 'tmux' from packages
Installing package 'tor' from packages
Installing package 'tracertools' from packages
Installing package 'transmission' from packages
Installing package 'travelmate' from packages
Installing package 'tree' from packages
Installing package 'triggerhappy' from packages
Installing package 'ttyd' from packages
Installing package 'tvheadend' from packages
Installing package 'u2pnpd' from packages
Installing package 'uanytun' from packages
Installing package 'udpxy' from packages
Installing package 'ulogd' from packages
Installing package 'umurmur' from packages
Installing package 'upmpdcli' from packages
Installing package 'usbip' from packages
Installing package 'usbmuxd' from packages
Installing package 'uuid' from packages
Installing package 'vallumd' from packages
Installing package 'vim' from packages
Installing package 'vncrepeater' from packages
Installing package 'vnstat' from packages
Installing package 'vpnbypass' from packages
Installing package 'vpnc' from packages
Installing package 'vsftpd' from packages
Installing package 'wakeonlan' from packages
Installing package 'watchcat' from packages
Installing package 'wavemon' from packages
Installing package 'webui-aria2' from packages
Installing package 'wget' from packages
Installing package 'wifidog' from packages
Installing package 'wifischedule' from packages
Installing package 'wifitoggle' from packages
Installing package 'xl2tpd' from packages
Installing package 'xupnpd' from packages
Installing package 'yaaw' from packages
Installing package 'youtube-dl' from packages
Installing package 'yunbridge' from packages
Installing package 'zabbix' from packages
Installing package 'zerotier' from packages
Installing package 'zile' from packages
Installing package 'zip' from packages
Installing package 'znc' from packages
Installing package 'zoneinfo' from packages
Installing package 'zsh' from packages
Installing all packages from feed luci.
Installing package 'community-profiles' from luci
Installing package 'freifunk-common' from luci
Installing package 'freifunk-firewall' from luci
Installing package 'freifunk-gwcheck' from luci
Installing package 'olsrd' from routing
Installing package 'freifunk-mapupdate' from luci
Installing package 'freifunk-p2pblock' from luci
Installing package 'freifunk-policyrouting' from luci
Installing package 'freifunk-watchdog' from luci
Installing package 'luci-app-adblock' from luci
Installing package 'luci-app-ahcp' from luci
Installing package 'ahcpd' from routing
Installing package 'luci-app-aria2' from luci
Installing package 'luci-app-asterisk' from luci
Installing package 'luci-app-commands' from luci
Installing package 'luci-app-coovachilli' from luci
Installing package 'luci-app-ddns' from luci
Installing package 'luci-app-diag-core' from luci
Installing package 'luci-app-diag-devinfo' from luci
WARNING: No feed for package 'smap' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'mac-to-devinfo' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'httping' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'smap-to-devinfo' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'netdiscover-to-devinfo' found, maybe it's already part of the standard packages?
Installing package 'luci-app-dump1090' from luci
Installing package 'luci-app-dynapoint' from luci
Installing package 'luci-app-freifunk-diagnostics' from luci
Installing package 'luci-app-freifunk-policyrouting' from luci
Installing package 'luci-app-freifunk-widgets' from luci
Installing package 'luci-mod-freifunk' from luci
Installing package 'luci-lib-json' from luci
Installing package 'luci-app-fwknopd' from luci
Installing package 'luci-app-hd-idle' from luci
Installing package 'luci-app-ltqtapi' from luci
Installing package 'luci-app-meshwizard' from luci
Installing package 'meshwizard' from luci
Installing package 'luci-app-minidlna' from luci
Installing package 'luci-app-mjpg-streamer' from luci
Installing package 'luci-app-mmc-over-gpio' from luci
Installing package 'luci-app-multiwan' from luci
WARNING: No feed for package 'multiwan' found, maybe it's already part of the standard packages?
Installing package 'luci-app-ntpc' from luci
Installing package 'luci-app-ocserv' from luci
Installing package 'luci-app-olsr' from luci
Installing package 'luci-lib-luaneightbl' from luci
Installing package 'luci-app-olsr-services' from luci
Installing package 'luci-app-olsr-viz' from luci
Installing package 'luci-app-openvpn' from luci
Installing package 'luci-app-p2pblock' from luci
Installing package 'luci-app-p910nd' from luci
Installing package 'luci-app-pbx' from luci
WARNING: No feed for package 'asterisk18' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-app-authenticate' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-app-disa' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-app-setcallerid' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-app-system' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-chan-gtalk' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-codec-a-mu' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-codec-alaw' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-func-cut' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-res-clioriginate' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-func-channel' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-chan-local' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-app-record' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-app-senddtmf' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'asterisk18-res-crypto' found, maybe it's already part of the standard packages?
Installing package 'luci-app-pbx-voicemail' from luci
WARNING: No feed for package 'asterisk18' found, maybe it's already part of the standard packages?
Installing package 'luci-app-polipo' from luci
Installing package 'luci-app-privoxy' from luci
Installing package 'luci-app-qos' from luci
Installing package 'luci-app-radicale' from luci
Installing package 'luci-app-radvd' from luci
WARNING: No feed for package 'radvd' found, maybe it's already part of the standard packages?
Installing package 'luci-app-rp-pppoe-server' from luci
Installing package 'luci-app-samba' from luci
Installing package 'luci-app-shadowsocks-libev' from luci
Installing package 'luci-app-shairplay' from luci
Installing package 'luci-app-shairport' from luci
Installing package 'luci-app-siitwizard' from luci
Installing package 'luci-app-splash' from luci
Installing package 'luci-app-statistics' from luci
Installing package 'luci-app-tinyproxy' from luci
Installing package 'luci-app-transmission' from luci
Installing package 'luci-app-travelmate' from luci
Installing package 'luci-app-udpxy' from luci
Installing package 'luci-app-unbound' from luci
Installing package 'luci-app-upnp' from luci
Installing package 'miniupnpd' from routing
Installing package 'luci-app-ushare' from luci
WARNING: No feed for package 'ushare' found, maybe it's already part of the standard packages?
Installing package 'luci-app-vnstat' from luci
Installing package 'luci-app-voice-core' from luci
Installing package 'luci-app-voice-diag' from luci
Installing package 'luci-app-vpnbypass' from luci
Installing package 'luci-app-watchcat' from luci
Installing package 'luci-app-wifischedule' from luci
Installing package 'luci-app-wol' from luci
Installing package 'luci-app-wshaper' from luci
WARNING: No feed for package 'wshaper' found, maybe it's already part of the standard packages?
Installing package 'luci-lib-httpclient' from luci
Installing package 'luci-lib-px5g' from luci
Installing package 'luci-lib-rpcc' from luci
Installing package 'luci-light' from luci
Installing package 'luci-mod-admin-mini' from luci
Installing package 'luci-theme-openwrt' from luci
Installing package 'luci-mod-failsafe' from luci
Installing package 'luci-mod-freifunk-community' from luci
Installing package 'luci-mod-rpc' from luci
Installing package 'luci-proto-3g' from luci
Installing package 'luci-proto-ipip' from luci
Installing package 'luci-proto-openconnect' from luci
Installing package 'luci-proto-qmi' from luci
Installing package 'luci-proto-relay' from luci
Installing package 'luci-proto-vpnc' from luci
Installing package 'luci-proto-wireguard' from luci
Installing package 'luci-ssl' from luci
Installing package 'luci-ssl-openssl' from luci
Installing package 'luci-theme-freifunk-generic' from luci
Installing package 'luci-theme-material' from luci
Installing package 'remote-update' from luci
Installing all packages from feed routing.
Installing package 'nat46' from routing
Installing package 'alfred' from routing
Installing package 'babel-pinger' from routing
Installing package 'babeld' from routing
Installing package 'batctl' from routing
Installing package 'batman-adv' from routing
Installing package 'batmand' from routing
Installing package 'bird' from routing
Installing package 'bird4-openwrt' from routing
Installing package 'bird6-openwrt' from routing
Installing package 'bmx6' from routing
Installing package 'bmx7' from routing
Installing package 'cjdns' from routing
Installing package 'hnetd' from routing
Installing package 'ohybridproxy' from routing
Installing package 'minimalist-pcproxy' from routing
Installing package 'luci-app-bmx6' from routing
Installing package 'luci-app-bmx7' from routing
Installing package 'luci-app-cjdns' from routing
Installing package 'mcproxy' from routing
Installing package 'mrd6' from routing
Installing package 'ndppd' from routing
Installing package 'nodogsplash' from routing
Installing package 'oonf-dlep-proxy' from routing
Installing package 'oonf-init-scripts' from routing
Installing package 'oonf-dlep-radio' from routing
Installing package 'oonf-olsrd2' from routing
Installing package 'pimbd' from routing
Installing package 'poprouting' from routing
Installing package 'quagga' from routing
Installing package 'smcroute' from routing
Installing package 'vis' from routing
Installing all packages from feed telephony.
Installing package 'asterisk-11.x' from telephony
Installing package 'dahdi-tools' from telephony
Installing package 'dahdi-linux' from telephony
Installing package 'libpri' from telephony
Installing package 'libsrtp' from telephony
Installing package 'iksemel' from telephony
Installing package 'asterisk-11.x-chan-dongle' from telephony
Installing package 'chan-sccp-b' from telephony
Installing package 'asterisk-13.x' from telephony
Installing package 'pjproject' from telephony
Installing package 'asterisk-g72x' from telephony
Installing package 'bcg729' from telephony
Installing package 'baresip' from telephony
Installing package 're' from telephony
Installing package 'rem' from telephony
Installing package 'spandsp' from telephony
Installing package 'freeswitch' from telephony
WARNING: No feed for package 'libsqlite2' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'flite' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libyuv' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libilbc' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libmemcached' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libsilk' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'libg7221' found, maybe it's already part of the standard packages?
WARNING: No feed for package 'freeradius-client' found, maybe it's already part of the standard packages?
Installing package 'kamailio-4.x' from telephony
Installing package 'ortp' from telephony
Installing package 'libosip2' from telephony
Installing package 'miax' from telephony
Installing package 'pcapsipdump' from telephony
Installing package 'restund' from telephony
Installing package 'rtpproxy' from telephony
Installing package 'sipp' from telephony
Installing package 'siproxd' from telephony
Installing package 'yate' from telephony
Installing all packages from feed management.
Installing package 'freecwmp' from management
Installing package 'libfreecwmp' from management
Installing package 'libmicroxml' from management
Installing package 'shflags' from management
Installing package 'freenetconfd' from management
Installing package 'freenetconfd-plugin-examples' from management
Installing package 'freesub' from management
Installing package 'libnetconf' from management
Installing package 'libssh' from management
Installing package 'libnetconf2' from management
Installing package 'libyang' from management
Installing package 'sysrepo' from management
Installing package 'netopeer2-cli' from management
Installing package 'netopeer2-server' from management
Installing package 'shtool' from management
Installing all packages from feed targets.
Installing all packages from feed cucumber.
Installing package 'socketman' from cucumber
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ 
```

In make menuconfig, go with defaults except for:

Network -> Socketman [M]
Image Configuration [*]) -> Separate feed repositories [*] -> Enable feed cucumber [*]
<Save> (writes to .config)
<Exit>
<Exit>

```
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ make menuconfig
Collecting package info: done
tmp/.config-package.in:51566:error: recursive dependency detected!
tmp/.config-package.in:51566:	symbol PACKAGE_iptables is selected by PACKAGE_vpnbypass
tmp/.config-package.in:76008:	symbol PACKAGE_vpnbypass depends on PACKAGE_ip
tmp/.config-package.in:56704:	symbol PACKAGE_ip is selected by PACKAGE_luci-mod-freifunk-community
tmp/.config-package.in:32165:	symbol PACKAGE_luci-mod-freifunk-community depends on PACKAGE_libuci
tmp/.config-package.in:30896:	symbol PACKAGE_libuci is selected by PACKAGE_firewall
tmp/.config-package.in:221:	symbol PACKAGE_firewall is selected by PACKAGE_luci-app-firewall
tmp/.config-package.in:32524:	symbol PACKAGE_luci-app-firewall is selected by PACKAGE_luci-app-p2pblock
tmp/.config-package.in:32819:	symbol PACKAGE_luci-app-p2pblock depends on PACKAGE_iptables


*** End of the configuration.
*** Execute 'make' to start the build or try 'make help'.

nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ 
```

```
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ make -j`grep -c processor /proc/cpuinfo`
 make[1] world
 make[2] tools/install
 make[2] package/cleanup
 make[3] -C tools/flock compile
 make[3] -C tools/flock install
 make[3] -C tools/tar compile
 make[3] -C tools/tar install
 make[3] -C tools/patch compile
 make[3] -C tools/expat compile
 make[3] -C tools/sstrip compile
 make[3] -C tools/make-ext4fs compile
 make[3] -C tools/firmware-utils compile
 make[3] -C tools/patch-image compile
 make[3] -C tools/expat install
 make[3] -C tools/sstrip install
 make[3] -C tools/make-ext4fs install
 make[3] -C tools/firmware-utils install
 make[3] -C tools/patch-image install
 make[3] -C tools/patch install
 make[3] -C tools/sed compile
 make[3] -C tools/m4 compile
 make[3] -C tools/xz compile
 make[3] -C tools/yaffs2 compile
 make[3] -C tools/cmake compile
 make[3] -C tools/scons compile
 make[3] -C tools/lzma-old compile
 make[3] -C tools/lzma compile
 make[3] -C tools/sed install
 make[3] -C tools/m4 install
 make[3] -C tools/pkg-config compile
 make[3] -C tools/xz install
 make[3] -C tools/mkimage compile
 make[3] -C tools/yaffs2 install
 make[3] -C tools/scons install
 make[3] -C tools/lzma-old install
 make[3] -C tools/lzma install
 make[3] -C tools/squashfs4 compile
 make[3] -C tools/autoconf compile
 make[3] -C tools/pkg-config install
 make[3] -C tools/mkimage install
 make[3] -C tools/squashfs compile
 make[3] -C tools/squashfs4 install
 make[3] -C tools/autoconf install
 make[3] -C tools/squashfs install
 make[3] -C tools/automake compile
 make[3] -C tools/missing-macros compile
 make[3] -C tools/automake install
 make[3] -C tools/missing-macros install
 make[3] -C tools/libtool compile
 make[3] -C tools/libtool install
 make[3] -C tools/gmp compile
 make[3] -C tools/libelf compile
 make[3] -C tools/flex compile
 make[3] -C tools/mklibs compile
 make[3] -C tools/e2fsprogs compile
 make[3] -C tools/mm-macros compile
 make[3] -C tools/gengetopt compile
 make[3] -C tools/patchelf compile
 make[3] -C tools/gmp install
 make[3] -C tools/libelf install
 make[3] -C tools/flex install
 make[3] -C tools/mklibs install
 make[3] -C tools/e2fsprogs install
 make[3] -C tools/mm-macros install
 make[3] -C tools/cmake install
 make[3] -C tools/patchelf install
 make[3] -C tools/mpfr compile
 make[3] -C tools/bison compile
 make[3] -C tools/mtd-utils compile
 make[3] -C tools/gengetopt install
 make[3] -C tools/mpfr install
 make[3] -C tools/mtd-utils install
 make[3] -C tools/mpc compile
 make[3] -C tools/bison install
 make[3] -C tools/findutils compile
 make[3] -C tools/bc compile
 make[3] -C tools/mpc install
 make[3] -C tools/bc install
 make[3] -C tools/findutils install
 make[3] -C tools/padjffs2 compile
 make[3] -C tools/quilt compile
 make[3] -C tools/padjffs2 install
 make[3] -C tools/quilt install
 make[2] toolchain/install
 make[3] -C toolchain/gdb prepare
 make[3] -C toolchain/binutils prepare
 make[3] -C toolchain/gcc/minimal prepare
 make[3] -C toolchain/kernel-headers prepare
 make[3] -C toolchain/musl/headers prepare
 make[3] -C toolchain/gcc/initial prepare
 make[3] -C toolchain/musl prepare
 make[3] -C toolchain/gcc/final prepare
 make[3] -C toolchain/fortify-headers prepare
 make[3] -C toolchain/gdb compile
 make[3] -C toolchain/binutils compile
 make[3] -C toolchain/kernel-headers compile
 make[3] -C toolchain/fortify-headers compile
 make[3] -C toolchain/binutils install
 make[3] -C toolchain/fortify-headers install
 make[3] -C toolchain/gcc/minimal compile
 make[3] -C toolchain/gdb install
 make[3] -C toolchain/gcc/minimal install
 make[3] -C toolchain/kernel-headers install
 make[3] -C toolchain/musl/headers compile
 make[3] -C toolchain/musl/headers install
 make[3] -C toolchain/gcc/initial compile
 make[3] -C toolchain/gcc/initial install
 make[3] -C toolchain/musl compile
 make[3] -C toolchain/musl install
 make[3] -C toolchain/gcc/final compile
 make[3] -C toolchain/gcc/final install
 make[2] target/compile
 make[3] -C target/linux compile
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.gz
 make[2] package/compile
 make[3] -C package/libs/toolchain compile
 make[3] -C package/system/usign host-compile
 make[3] -C package/kernel/gpio-button-hotplug compile
 make[3] -C package/firmware/linux-firmware compile
 make[3] -C package/firmware/prism54-firmware compile
 make[3] -C package/firmware/b43legacy-firmware compile
 make[3] -C package/libs/mbedtls compile
 make[3] -C package/network/ipv6/odhcp6c compile
 make[3] -C package/network/services/dnsmasq compile
 make[3] -C package/network/services/dropbear compile
 make[3] -C package/libs/libpcap compile
 make[3] -C package/network/utils/linux-atm compile
 make[3] -C package/network/utils/resolveip compile
 make[3] -C package/utils/busybox compile
 make[3] -C package/libs/libnl-tiny compile
 make[3] -C package/libs/libjson-c compile
 make[3] -C package/utils/lua compile
 make[3] -C package/libs/lzo compile
 make[3] -C package/libs/zlib compile
 make[3] -C package/utils/util-linux compile
 make[3] -C package/boot/uboot-ar71xx compile
 make[3] -C package/libs/libnl compile
 make[3] -C package/libs/openssl compile
 make[3] -C package/libs/polarssl compile
 make[3] -C feeds/packages/libs/c-ares compile
 make[3] -C package/kernel/linux compile
 make[3] -C package/network/utils/iw compile
 make[3] -C package/network/utils/iptables compile
 make[3] -C package/network/services/ppp compile
 make[3] -C package/libs/libubox compile
 make[3] -C package/utils/ubi-utils compile
 make[3] -C package/network/utils/curl compile
 make[3] -C feeds/packages/net/mosquitto compile
 make[3] -C package/libs/ustream-ssl compile
 make[3] -C package/system/mtd compile
 make[3] -C package/system/ubus compile
 make[3] -C package/system/uci compile
 make[3] -C package/utils/jsonfilter compile
 make[3] -C package/system/usign compile
 make[3] -C package/network/services/hostapd compile
 make[3] -C package/libs/uclient compile
 make[3] -C package/network/config/firewall compile
 make[3] -C package/network/config/swconfig compile
 make[3] -C package/network/services/odhcpd compile
 make[3] -C package/network/utils/iwinfo compile
 make[3] -C package/system/opkg compile
 make[3] -C package/network/config/netifd compile
 make[3] -C package/system/ubox compile
 make[3] -C feeds/cucumber/socketman compile
 make[3] -C package/kernel/mac80211 compile
 make[3] -C package/system/fstools compile
 make[3] -C package/system/procd compile
 make[3] -C package/boot/uboot-envtools compile
 make[3] -C package/base-files compile
 make[2] package/install
 make[3] -C package/system/opkg host-install
 make[3] package/preconfig
 make[2] target/install
 make[3] -C target/linux install
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.elf
 make[6] -C target/linux/ar71xx/image/lzma-loader compile loader.elf
 make[2] package/index
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ 
```

```
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt$ cd bin/ar71xx/packages/cucumber
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt/bin/ar71xx/packages/cucumber$ ls -l
total 48
-rw-r--r-- 1 nbowden nbowden   511 Feb  6 06:39 Packages
-rw-r--r-- 1 nbowden nbowden   363 Feb  6 06:39 Packages.gz
-rw-r--r-- 1 nbowden nbowden   151 Feb  6 06:39 Packages.sig
-rw-r--r-- 1 nbowden nbowden 32896 Feb  6 06:21 socketman_0d94378_ar71xx.ipk
nbowden@nbowden-Aspire-E1-532:~/dev/openwrt/bin/ar71xx/packages/cucumber$ 
```



  

