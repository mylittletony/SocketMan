# SocketMan

A light-weight daemon, written purely in c, that runs on most OpenWRT, LEDE and other \*nix devices. It has few dependencies to keep it as light as possible.

It will create a connection to an MQTT broker and await instruction. It monitors the network and will attempt to recover itself in case of emergency. Every minute or so, it runs a collection job to grab as many wireless stats as it can. It then posts these back to an API end-point as JSON.

This project is NOT complete - it functions, only in the basic form. It should be in Beta at the end of September 2016.

Full build instructions will follow. It currently runs on Debian, Ubuntu and OpenWRT. And LEDE probably. Debian requires sudo to perform a full scan.

## TODO

- Set periodic scan
- Cache and send json
- Implement protocol buffers
- Implement certificate refresh

## Notes

To obtain the IP and device name for a station, we currently read the DNSMASQ lease file. If it's not present, we can't get the IP, yet. We're working on something to bypass this.

## JSON Formats

**Operations**

SocketMan can notify a RESTful API of the progress of an operation. Use the following JSON format if using Puffin. If REST is disabled, the state of an operation is conveyed via MQTT.

Type: PATCH
URL: https://api.puffin.ly/v1/operations/{operation_id}?access_token={device_public_token}
OPERATION_ID: the ID of the op, as sent with a message
REQUIRED: access_token -> public token for a device


```
{
    "success": true,
    "output": "command output"
}
```
