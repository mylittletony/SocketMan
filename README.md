# SocketMan

A light-weight daemon, written purely in c, that runs on most OpenWRT, LEDE and other \*nix devices. SocketMan is used at Cucumber Tony to control their global estate of access points and routers.

The project is still being developed. A full beta release will be made available soon.

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

By default, SocketMan will log via syslog. If you want to change this behaviour, you can add the environment variable ```DEBUG``` which will force it to log to the terminal.

For example:

```
DEBUG=1 socketman --config=/etc/config.json
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
- heartbeat, integer.  Default 60. How long to wait before sending data.
- debug, bool. Default 0. Enable debug mode.
- scan, integer. Run the stats collection, or not. Default 1.
- no-cache, integer. If disabled, send pure JSON stats
- no-ping, integer. If disabled, SocketMan won't run a ping test, RESTFully or via MQTT
- ping-interval, integer. Default 180. Sends a ping via MQTT and POST via api_url.
- rest, integer. Default 0. Enabling will send job notifications via a POST. Otherwise send via MQTT.
- reboot, integer. Default 600. How long to wait before rebooting. Disable with 0.
- health_port, integer. Default 53. The port to check for internet connectivity.
- health_url, string. The default URL to use to check for internet connectivity.
- qos, integer. MQTT QoS. 0, 1 or 2. Default 0.
- insecure, integer. Default 0. Send API requests without any certificate validation.
- backup_stats_url, string. Default empty. Try a secondary URL if stats POST fails.
- boot_url, string. Default empty. Call a URL on boot.
- boot_cmd, string. Default empty. Run a script on boot.
- mac_file, string. Default /etc/mac. The file containing your default WAN MAC.

## Initializing the devices

By default, the devices will initialise themselves against the Cucumber API. If you want to change this behaviour, you can run with the -b flag followed by your init. server URL. For example:

```
socketman --config=/etc/config.json -b https://api.acme-corp.com/init
```

## Sending jobs to devices

Ignore this if using with Cucumber. This applies only if you're using your own MQTT broker.

SocketMan creates a secure tunnel via MQTT to an MQTT broker. The job must be of the following JSON format.

```
{
	"id": "your-job-id",
	"timestamp": 1484484398,
	"meta": {
		"msg": "your-command",
		"type": "your-job-type"
	}
}
```

Store the command you want to run in the meta.msg key. An ID is required and used to identify the job upon success or fail.

The type key is special. If you send a job with type 'network', SocketMan will perform a network connectivity test before and after the job run. In the event of failure, SM will recover.

Publish the job to the topic as so:

```
topic := "sub/" + your-topic + "/" + your-key + "/" + your-mac
```

Where your-topic, your-key and your-mac are defined in the config.json:

```
sub/my-topic/my-key/11:22:33:44:55:66
```

If you have enabled rest via the options, SocketMan will notify your API whether the job was a success or not. It will also include the output of the command if required.

With rest disabled (the default), SocketMan will publish a job back to your MQTT broker instead. This will be published to the following topic:

```
topic := "pub/" + your-topic + "/" + your-key + "/" + your-mac
```

The JSON will be in the following format:

```
{
	"id": "your-job-id",
	"timestamp": 1484484398,
	"app": "socketman",
	"event_type": "PROCESSED",
	"meta": {
		"msg": "command-output",
		"type": "your-job-type"
	}
}
```

For example, if you publish a job with the msg: uptime. SocketMan will process the job and publish the output back to your servers.

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

The collector runs every 15 seconds. It caches the data and POSTs to your API as GZIPPED file. The file contains the JSON collection data shown below. Each entry is appended as a new line.

The JSON is quite long so a snippet can be found here:

https://gist.github.com/simonmorley/74f97ba9d267f5eb9eea5c43490ca337

## Building SocketMan

An opkg will be available as soon. In the short-term, you can build SocketMan manually. Consult the examples directory.

## Support

Commercial support is available via Cucumber Tony, please contact the team via your dashboard (https://dashboard.ctapp.io).

Community support is available via the Cucumber discussions:

https://discuss.cucumberwifi.io/c/socketman-openwrt-broker

## Notes

To obtain the IP and device name for a station, we currently read the DNSMASQ lease file. If it's not present, we can't get the IP, yet. We're working on something to bypass this.
