# Robus HAL Mockup

This mockup is intended to let you test the robus library behavior without needing to use specific hardware.

Each software module can communicate with the other through the mockup hal. All modules are first registered to a [broker](broker.py) which role is to broadcast the messages to all connected robus modules. The transport layer is implemented using the [zmq](http://zeromq.org) library.

## Use example

* First, run the broker. The usual *python broker.py* should do the trick.
* Then, you can create program test using this hal. An example is given in [this test](../../../extra/examples/mockup/test.c).

The robus_* functions should work as expected.
