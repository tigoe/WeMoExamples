# WeMo Examples

Examples and notes for controlling [Belkin WeMo devices](https://www.belkin.com/us/Products/home-automation/c/wemo-home-automation/) from an Arduino.

There are many different examples of how to contro a WeMo using a variety of programming tools -- [node.js](https://www.npmjs.com/package/wemo-client), [python](https://ouimeaux.readthedocs.io/en/latest/#), [groovy](https://objectpartners.com/2014/03/25/a-groovy-time-with-upnp-and-wemo/), etc. [OpenRemote](https://github.com/openremote/Documentation/wiki/Belkin-WeMo) has some useful notes as well. But I couldn't find good examples for  Arduino, so I wrote [an example for the MKR1000](https://github.com/tigoe/MakingThingsTalk2/tree/master/3rd_edition/chapter9/WemoHttpClient) for [Making Things Talk](https://www.makingthingstalk.com) a few years ago. This repository expands on that, adding an example for how to read the [WeMo Insight](https://www.belkin.com/us/p/P-F7C029/). It may expand to other WeMo devices in the future. 

This repository will also include any notes I've picked up along the way, for future use.

## WeMo Setup

When you first plug in the WeMo, it will set itself up as a WiFi access point (AP).  You connect to the WeMo, then use the [WeMo app for Android](https://play.google.com/store/apps/details?id=com.belkin.wemoandroid&hl=en_US) or [Wemo app for iOS](https://itunes.apple.com/us/app/wemo/id511376996?mt=8) to do the initial setup for your WeMo. You'll need to make sure your Android or iOS device is on the network to which you want to connect the WeMo, as the app will pass in the credentials. 

### Getting the WeMo Mac Address

If your network uses MAC address filtering as many academic and commercial networks do, you'll need to register the MAC address of your WeMo device  on your network before it will successfully connect. This can be tricky, as the WeMo Insight switches use two MAC addresses. The one printed on the device is the one they use to create the initial AP, but when they connect to another network, they change to a MAC address that's one value higher. For example, my WeMo Insight's printed MAC address ends in `95:e4`, but the address when it's on my network ends in `95:e5`. If you can pre-register both addresses on your network, that can be helpful. 

Not all WeMo devices exhibit this MAC address switching behavior. For example, the WeMo Switch Mini appears to stick with a single address.

### Making an Alternate Setup Interface

...is not yet done.

WeMo devices communicate using the [Universal Plug & Play](https://en.wikipedia.org/wiki/Universal_Plug_and_Play) (UPnP) protocol over HTTP on port 49153. uPnP supports device discovery and service advertisement and all sorts of other things, so it should be possible to configure a WeMo without the app. But so far, no one's done it, and I haven't bothered either. The most detail I've found is on this [WeMo communications analysis](https://www.scip.ch/en/?labs.20160218). These folks sniffed the initial data exchange, but didn't work out the password encryption. It looks like it's encrypted using [TKIP](https://en.wikipedia.org/wiki/Temporal_Key_Integrity_Protocol), but I haven't looked into that yet. Assuming you could encrypt a password, you could make a custom application to set up WeMo devices.

## WeMo Switch Control

To control a WeMo switch, send an HTTP POST request to port 49153. The on/off control request looks like this:

````
POST /upnp/control/basicevent1 HTTP/1.1
Host: 192.168.0.13:49153
Content-type:text/xml;  charset=utf-8
SOAPACTION:"urn:Belkin:service:basicevent:1#SetBinaryState"
Content-Length: 302

<?xml version="1.0" encoding="utf-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"> <s:Body><u:SetBinaryState xmlns:u="urn:Belkin:service:basicevent:1"> <BinaryState>1</BinaryState></u:SetBinaryState></s:Body></s:Envelope>
````

To make this request using curl, you'd do this:

````
curl -H 'Content-type:text/xml;  charset=utf-8' -H 'SOAPACTION:"urn:Belkin:service:basicevent:1#SetBinaryState"' -d '<?xml version="1.0" encoding="utf-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"> <s:Body><u:SetBinaryState xmlns:u="urn:Belkin:service:basicevent:1"> <BinaryState>1</BinaryState></u:SetBinaryState></s:Body></s:Envelope>' 'http://192.168.0.13:49153/upnp/control/basicevent1'
````
By changing the value inside the `<BinaryState></BinaryState>` tag, you can switch the WeMo from off to on. You can see this in action in the [WemoHttpClient](WemoHttpClient/WemoHttpClient.ino) example. 

From a WeMo Switch Mini, you'll get a response like this:

````
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body>
<u:SetBinaryStateResponse xmlns:u="urn:Belkin:service:basicevent:1">
<BinaryState>1</BinaryState>
<CountdownEndTime>0</CountdownEndTime>
<deviceCurrentTime>1549126684</deviceCurrentTime>
</u:SetBinaryStateResponse>
````

And from a WeMo Insight, you'll get a response like this:

````
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body>
<u:SetBinaryStateResponse xmlns:u="urn:Belkin:service:basicevent:1">
<BinaryState>8|1549126343|0|0|0|8039|10|0|0|0</BinaryState>
<CountdownEndTime>0</CountdownEndTime>
<deviceCurrentTime>1549126754</deviceCurrentTime>
</u:SetBinaryStateResponse>
````

## Reading a WeMo Insight

The WeMo Insight can give you a reading of its energy usage. To get this, you make a request for the `InsightParams` property, and you get a response with values in pipe-delimited form. Here's the POST request you'd make:

````
POST /upnp/control/insight1 HTTP/1.1
Host: 192.168.0.14:49153
Content-type:text/xml;  charset=utf-8
SOAPACTION:"urn:Belkin:service:insight:1#GetInsightParams"
Content-Length: 271

<?xml version="1.0" encoding="utf-8"?><s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><u:GetInsightParams xmlns:u="urn:Belkin:service:insight:1"></u:GetInsightParams></s:Body></s:Envelope>
````

And here's the curl command:

````
curl -H 'Content-type:text/xml;  charset=utf-8' -H 'SOAPACTION:"urn:Belkin:service:insight:1#GetInsightParams"' -d '<?xml version="1.0" encoding="utf-8"?><s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><u:GetInsightParams xmlns:u="urn:Belkin:service:insight:1"></u:GetInsightParams></s:Body></s:Envelope>' 'http://192.168.0.14:49153/upnp/control/insight1'
````

The response looks like this:

````
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body>
<u:GetInsightParamsResponse xmlns:u="urn:Belkin:service:metainfo:1">
<InsightParams>8|1549126755|0|0|0|9319|10|0|0|0.000000|7000</InsightParams>
</u:GetInsightParamsResponse>
````

The pipe-delimited string inside the `<InsightParams>` tag is the data you want. Though Belkin doesn't publish what the parameters are anymore, they are listed on [the Eclipse.org Smarthome site](https://www.eclipse.org/smarthome/documentation/features/bindings/wemo/readme.html). They are as follows:

* state - Whether the switch is on or off (1 or 0).
* lastChangedAt - The date and time when the WeMo was last turned on or off (a Unix timestamp)
* lastOnFor - How long the Insight was last on for (seconds).
* onToday -  How long the Insight has been on today (seconds)'
* onTotal - How long the Insight has been on total (seconds).
* timespan - Timespan over which onTotal is relevant (seconds). Typically 2 weeks except when first started up.
* averagePower - Average power consumption (Watts).
* currentPower - Current power consumption (milliwatts).
* energyToday - Energy used today (Watt-hours, or Wh).
* energyTotal - Energy used in total (Wh). This is the only parameter reported as a floating point value. 
* standbyLimit - Minimum energy usage to register the insight as switched on ( milliwats, default 8000mW, configurable via WeMo App).

You can see this in action in the [WemoInsightHttpClient](WemoInsightHttpClient/WemoInsightHttpClient.ino) example.
