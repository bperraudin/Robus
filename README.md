# Robus
Robus, the robust **RO**bot **BUS**

Building robots is really cool but can become really complex, you will need strong knowledge in electronics design, mechanical design, and both low level and high level programming.

**Pollen Robotics wants to simplify it.**

## Robus objectives

We want to allow you to build your own robot according to your level and experience. This is made possible by Robus, our unified modular architecture dedicated to robotics.

Robus reduces the time between an idea to the prototype. It provides a unified messaging achitecture for modular robotics, all modules can be connected on the same 5 wires bus containing both power and 2 communication bus. The network can self-discover module and their topology. Robus differentiating attributes are:
- TODO
- TODO
- TODO

TODO: Explain this better: "You can manage you robot using Arduino style low level code, high level language such as python, or both to have the perfect combination between performance and mantainability."

### Connection to the outside world

Robus is designed to be connected to many systems or devices. You will be able to control your robot via Wifi or Bleutooth using your computer or your tablet. Robus will also be able to connect with other Robus based robots or others complemetary achitectures like Lego Mindstorm or Robotis motors.

### Extendable

If the technology you need is not already available on the Robus module list, no problem, Robus allows you to design your own by simply conforming to our API and guidelines. 

TODO: A bit more details on what we need to do
TODO: Link to API/guidelines

## How do I use Robus?

> I just want to test my idea by building a functional robot quikly.

You just need to use standard modules and one of our high level programing interfaces such as Scratch or Python.

Follow [this tutorials](extra/doc/beginners.md) to learn how.

> I want to create my robot and master each and every details of the robot's behavior.

In addition to our high level programming interfaces, you will need to modify some low level embeded code in the modules. Robus allows you to create low level behavior localized on each specific module. This is vastly facilitated by the Robus messaging system freeing you from the cumbersome protocol writting process and module/service identification.

Follow [this tutorial](extra/doc/intermediates.md) to learn how.

> I want to develop a new sensor or actuator, can I prototype quickly a new module.

TODO: Prottyping board will be available with the chip embeded by easy access to the pins...

Follow [this tutorial](extra/doc/advanced.md) to learn how.

> My needs are even more specific, I want to develop mass-produced modules.

You will need to understand how Robus works and can be integrate in new electronics designs, fortunatly you will see great help in our debugging and sniffing tools.

Follow [this tutorial](extra/doc/developers.md) to learn how.

## Installation

On Mac you will nedd to install 2 more things : 
- `brew install coreutils`
- `brew install crosstool-ng`
