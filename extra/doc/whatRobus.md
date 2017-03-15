# What is Robus?
Robus is an hardware module based system dedicated to robot creation. In your robot each functional bloc will be considered as modules. 

## What can I do with Robus?
With Robus each components of your robot can host an inteligence, a module is a smart component. Robus allow you to put your own code on any module to create your behaviors accordingly to your robot need.
For example your motor module can manage your specific angle curv, acceleration or record and play a movement by itself. You just have to create a behavior and put it into your device.

Each modules use the same hardware and software interface allow them to be compatible each other. Modules can be chained to avoid thousand of cable on your robot but you also can plug them in parallel using hub modules. When multiple module are linked together they share the same power source and can communicate together to share theire capabilities.

Modules can detect each other and take an unic address on your robot network depending of the wiring position. If you want to change a sensor for a better one you just have to replace it and it take the same address number. You also can give readable name to your module to easily identify them on a complex robot. This alias name will be kept if you use it on another robot but you can modify it as you want.

Each module can take the lead for a moment to send information or control others modules. With this features combined to the possibility to be programed with your custom behaviors, you can create amazing complex reaction:
> if battery is low ask to the speaker module to play the "batterie_low_alert" song.

> if camera detect movement send a pictures to a server using the wifi module.

> If camera detect a banana take it using arm motors!

A phisical module can have multiple virtual modules inside. For exemple if we have a line folower sensor composed with 2 infrared sensors we can separate this physical module in 2 infrared sensor virtual modules with two different unic ID.

When your robot is created you can update behaviors without unmounting the robot to access your module, you can flash it using specific Robus modules called Gates modules.

## Robus give you some tools to satisfy all kind of creation desire:
- The arduino compatible developpement board can be used with all arduino uno shield, sensors, and libs. So you can easily evolve your arduino creations with Robus modules or create your realy custom Robus based modules.
- The Robus debug module allow your to catch any message on your robot to understand what's appening in your robot.
- The Robus library allow you to create your module from scratch