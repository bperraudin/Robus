# Robus
Robus, the robust **RO**bot **BUS**

Building robots is really cool but can become really complex, you will need strong knowledge in electronics design, mechanical design, and both low level and high level programming.

**Pollen Robotics wants to simplify it.**

## Robus objectives

We want to allow you to build your own robot according to your level and experience. This is made possible by Robus, our unified modular architecture dedicated to robotics.

Robus reduces the time between an idea to the prototype. It provides a unified messaging achitecture for modular robotics, all modules can be connected on the same 5 wires bus conatining both power and 2 communication bus. The network can self-discover module and their topology. Robus differentiating attributes are:
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

> I just want to realize my ideas by creating my robot quikly.

You will probably just need to use standard module and high level programing interface such as Scratch or Python.
Follow [this tutorials](extra/doc/beginers.md).

> I want to create my robot and mastering with presision his behavior.

You will need to combine high level programming langage such as Scratch or Python and low level embeded code.
Robus allow you to create low level behavior localized on specific module and acess it easily using high level languages.
Follow [this tutorial](extra/doc/experimented.md) to learn how.

> My needs are very specific, I want to make my own modules.

You will need to understand how to create your hown module and how Robus work for debugging.
Follow [this tutorial](extra/doc/developers.md).



On Mac you will nedd to install 2 more things : 
`brew install coreutils`
`brew install crosstool-ng`
