
# Hue Dash

### What's this?

Abuse those inexpensive [Amazon Dash Buttons](https://en.wikipedia.org/wiki/Amazon_Dash) to control your Hue Lights (or anything else, really). There are many different programs doing this already, but this one is especially lightweight and free of dependencies since it's only a very short C program which then invokes `curl` to talk to the Hue bridge.

![Button](https://raw.github.com/blitzcode/hue-dash/master/button.gif)

### What Do I Need?

- Amazon Dash (~5EUR)
- Always-on Linux computer with a C compiler & curl, Raspberry Pi will do (~25-50EUR)

### Setup

Setup your buttons like in the official guide, but abort setup before selecting a product to order. This way your button can connect to your WiFi but won't be able to order anything from Amazon.

Next step is configuring the program source (`main.c`) with the details of your button(s), Hue bridge and light(s). You'll need to obtain the [MAC address](https://en.wikipedia.org/wiki/MAC_address) of each button (look in the device list of your router after the button is configured), the IP / host name of your Hue bridge, a whitelisted user ID for the Hue API and the light IDs for the lights you want to switch. If you don't know how to do that, read the [Getting Started](http://www.developers.meethue.com/documentation/getting-started) section of the Hue API documentation (registration required). Modify the `set_light_state()` function at the top with your Hue details, then modify the MAC addresses and actions at the very bottom.

Then, compile the modified program.

    make

That should do it. The program watches for an [ARP](https://en.wikipedia.org/wiki/Address_Resolution_Protocol) message from the button that is sent when it wakes up after a button press and reconnects to your WiFi. We need super user privileges to be able to snoop on ARP traffic. Run with

    sudo ./out

Since the program needs to be always running, it would be a good idea to run it as a daemon. I use [daemontools](https://cr.yp.to/daemontools.html), also see [this](https://info-beamer.com/blog/running-info-beamer-in-production) great tutorial.

# Legal

This program is published under the [MIT License](http://en.wikipedia.org/wiki/MIT_License).

# Author

Developed by Tim C. Schroeder, visit my [website](http://www.blitzcode.net) to learn more.

