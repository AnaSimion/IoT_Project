# IoT_Project

### Set Up

You will need to install the [Pistache](https://github.com/pistacheio/pistache) library.
On Ubuntu, you can install a pre-built binary as described [here](http://pistache.io/docs/#installing-pistache).

### MQTT

I use MQTT Explorer: http://mqtt-explorer.com/

For MQTT protocol support, you can use the Eclipse [Paho](https://www.eclipse.org/paho) client library. The [Paho C++ library](https://github.com/eclipse/paho.mqtt.cpp#unix-and-linux) can be installed from source (will require you to also build and install the Paho C library).

MQTT also requires a server (a message broker) to be running in order to have where to send the messages. For this, you can use Eclipse [Mosquitto](https://mosquitto.org/). On Ubuntu, it can be installed from a PPA:

```sh
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
sudo apt update
sudo apt install mosquitto
```

A step by step series of examples that tell you how to get a development env running

You should open the terminal, navigate into the root folder of this repository, and run\

g++ -std=c++17 car_example.cpp -o car -lpistache -lcrypto -lssl -lpthread -lpaho-mqttpp3 -lpaho-mqtt3a

This will compile the project using g++, into an executable called `car` using the libraries `pistache`, `crypto`, `ssl`, `pthread`. You only really want pistache, but the last three are dependencies of the former.
Note that in this compilation process, the order of the libraries is important.

To test, open up another terminal, and type\
`curl http://localhost:9080/ready`

Number 1 should display.
Now you have the server running.

Routes example:

Get:
http://localhost:9080/settings/temperature

Set:
http://localhost:9080/sensorName/revision_done/25000

We also use Postman for route navigation: https://www.postman.com/
