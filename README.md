# SocketMan

A light-weight daemon, written purely in c, that runs on most OpenWRT, LEDE and other \*nix devices. It has few dependencies to keep it as light as possible.

It will create a connection to an MQTT broker and await instruction. It monitors the network and will attempt to recover itself in case of emergency. Every minute or so, it runs a collection job to grab as many wireless stats as it can. It then posts these back to an API end-point as JSON.

This project is NOT complete - it functions, only in the basic form. It should be in Beta at the end of September 2016.

Build instructions will follow.

A separate license is included.
