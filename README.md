# SocketMan

A light-weight daemon, written purely in c, that runs on most OpenWRT, LEDE and other \*nix devices. It has few dependencies to keep it as light as possible.

It will create a connection to an MQTT broker and await instruction. It monitors the network and will attempt to recover itself in case of emergency. Every minute or so, it runs a collection job to grab as many wireless stats as it can. It then posts these back to an API end-point as JSON.

This project is NOT complete - it functions, only in the basic form. It should be in Beta at the end of September 2016.

Full build instructions will follow. It currently runs on Debian, Ubuntu and OpenWRT. And LEDE probably. Debian requires sudo to perform a full scan.

**TODO**

- Collect DHCP entries
- Collect captive portal connections
- Process inbound commands
- Run program on boot
- Run cURL on boot
- Set periodic scan
- Cache and send json
- Implement protocol buffers
- Implement certificate refresh

**Notes**

To obtain the IP and device name for a station, we currently read the DNSMASQ lease file. If it's not present, we can't get the IP, yet. We're working on something to bypass this.
