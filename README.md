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

## Config

Place a JSON config file with all the required params:

```
{
  "username": "hXwYkJrhgt",
  "password": "ZaXMATRyQn",
  "topic": "ZsSChvluYyGCNRbWHgDKAQbvBLeozVuq",
  "key": "SObjvWxbQtcotVrQJWurPcvnPXmpqYue",
  "api_url": "http://xxx.ngrok.io/v1/collector",
  "stats_url": "http://xxx.ngrok.io/v1/collector",
  "backup_url": "http://requestb.in/1n69hks1",
  "mac": "88-DC-XX-XX-XX-XX",
  "mqtt_host": "192.168.142.139",
  "booiiiit_url": "123",
  "booiit_cmd": "/tmp/test.sh",
  "token": "75199254-5ab0-4fce-a8fc-e16b1978b103",
  "port": 8443,
  "sleep": 10,
  "monitor": 5,
  "scan": 1,
  "survey": 0,
  "tls": true,
  "cacrt": "/tmp/cacrt.pem",
  "debug": 1
}
```

You can either get directly from the CT API or SM will provision itself on boot.

Run SocketMan with:

```
socketman --config=/root/config.json
```

## JSON Formats

**Operations**

SocketMan can notify a RESTful API of the progress of an operation. Use the following JSON format if using Puffin. If REST is disabled, the state of an operation is conveyed via MQTT.

- METHOD: PATCH
- URL: https://api.puffin.ly/v1/operations/{operation_id}?access_token={device_public_token}
- OPERATION_ID: the ID of the op, as sent with a message
- REQUIRED: access_token -> public token for a device
- TYPE: application/json

And here's what the JSON looks like.

```
{
    "success": true,
    "output": "command output"
}
```

**Collector**

The collector runs every N seconds. It reports stats on the boxes and pushes them back to Tony. If you're not using the REST API, it will publish back to Puffin.

The JSON is quite long so a snippet can be found here:

https://gist.github.com/simonmorley/74f97ba9d267f5eb9eea5c43490ca337

## Notes

To obtain the IP and device name for a station, we currently read the DNSMASQ lease file. If it's not present, we can't get the IP, yet. We're working on something to bypass this.
