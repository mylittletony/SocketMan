# SocketMan

A light-weight daemon, written purely in c, that runs on most OpenWRT, LEDE and other \*nix devices. SocketMan is used at Cucumber Tony to control their global estate of access points and routers.

The project is still being devoloped. A full beta release will be made avaialable soon.

It performs the following functions:

- It monitors network connectivity.
- Collects, caches and sends device stats to your chosen API. 
- Connects via MQTT to your broker and processes inbound jobs. 
- Recovers from network failure.

## Config

Create a config.json file as so:

```
{
  "username": "hXwYkJrhgt",
  "password": "ZaXMATRyQn",
  "topic": "ZsSChvluYyGCNRbWHgDKAQbvBLeozVuq",
  "key": "SObjvWxbQtcotVrQJWurPcvnPXmpqYue",
  "api_url": "http://api.my-domain.io/v1/collector",
  "stats_url": "http://api.my-domain.io/v1/collector",
  "backup_url": "http://requestb.in/1n69hks1",
  "mac": "88-DC-XX-XX-XX-XX",
  "mqtt_host": "192.168.142.139",
  "token": "75199254-5ab0-4fce-a8fc-e16b1978b103",
  "port": 8443,
  "sleep": 60,
  "monitor": 15,
  "scan": 1,
  "survey": 0,
  "tls": true,
  "cacrt": "/tmp/cacrt.pem"
}
```

We recommend saving this outside the configs directory since that is monitored by SocketMan.

If you're using this with Cucumber, the device will download the file automatically and save in your specified folder. The device must be added directly to the Cucumber dashboard.

## Running it

```
socketman --config=/etc/config.json
```

## Config Options

The many options available to you via the config.json file. 

- username, string. MQTT username
- password, string. MQTT password
- topic, string. MQTT topic
- key, string. MQTT key
- port, integer. The MQTT port number.
- tls, boolean. Enable or disable MQTT tls options.
- cacrt, string. MQTT server CA.
- mqtt_host, string. MQTT broker hostname.
- api_url, string. Can be used for provisioning, job notification etc.
- stats_url, string. The API you're using for stats processing.
- mac, string. The MAC of the device.
- token, string. Your public API token. Will be appeneded to all API requests
- monitor, integer. How often to run the collection. Min 15 seconds.
- sleep, integer. How long to wait before sending data. Default 60.
- debug, bool. Enable debug mode.
- scan, integer. Run the stats collection, or not. Default 1.
- no-cache, integer. If disabled, send pure JSON stats

## JSON Specification

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

### Collector

The collector runs every 15 seconds. It caches the data and POSTs to your API as GZIPPED file. The file contains the JSON collection data shown below. Each entry is appeneded as a new line.

The JSON is quite long so a snippet can be found here:

https://gist.github.com/simonmorley/74f97ba9d267f5eb9eea5c43490ca337

## Notes

To obtain the IP and device name for a station, we currently read the DNSMASQ lease file. If it's not present, we can't get the IP, yet. We're working on something to bypass this.
