Protocole definition
====================

Hardware and data flow
----------------------

Most Poppy modules have two communication ports that can be daisy-chained.

The Poppy communication interface uses 3 data wires:

 - RS485+
 - RS485-
 - PTP

Wire RS485+ and RS485- form a half-duplex serial bus using **RS485** that is shared by all the connected devices. This bus is the main communication way. The **PTP** wire means "point to point", it is used for hardware detection and allows direct communicatation with the next or previous module.

Each of these links (PTP or RS485) can be used to communicate.

Each module has a standard **Poppy-core** board at heart, which manages all the poppy-com things and can communicate with, control and flash a companion MCU specific to each module hardware.

All messages RS485 or PTP have the same structure:

 - **Preamble** (N bytes duration) 
 - **Target**  (16 bits)
 - **Source**  (16 bits)
 - **Msg_type**  (13 bits)
 - **Ack_enable** (1 bit)
 - **Msg_size** (10 bits) 
 - **Msg [0]** (8 bits)
 - **Msg [1]** (8 bits)
 - **...** 
 - **Msg [Msg_size-1]** (8 bits) 
 - **CRC** (16 bits)

Then, if ack_enable is set, the target should respond, less than N bytes duration after the CRC: 
 - **Ack** or **Nack**  (8 bits)



Preamble
--------

The **Preamble** is an event that preceed any message. Each transmitter need to respect this **Preamble** before any transmission.
The poppy-com **Preamble** is a complete innactivity on the bus during aperiod of time.

(**Tpreamble** =  N x byte + random)

A transmitter can only send something after this step.
The timeout time (**Ttimeout**) of a poppy-com message is lower than the **Preamble** time to avoid any message read overlaping.

**Ttimeout** < **Tpreamble**

Target / source
---------------

**Target** and **Source** indicate who talk to who. Each module can be dynamically addressed on a network.
Each module have an unic identifier (**UniqueID**) that allow any other module to speek with him. Some modules can have unic identifier (**UniqueID**) and multiples virtual identifier (**VirtualID**) to seperate multiple device through the same module (for example a poppy-com2AX with many dinamixel motor can give an unic identifier for each motor, or a dev board with multiple sensors...).

The **Target** field could also be used as multicast identifier. The multicast identifier can be listened/managed by multiple modules or virtuals modules.

Msg type
--------

The **Msg_type** field is used to say the topic of the message. For a motor for example you can have the speed **Msg_type**, the position **Msg_type**, ...

Msg size
--------

This field give the size (in bytes) of the following datas. A data can have 1023 bytes max.

Msg[X]
-----

This field contain datas.

CRC
---

This field is generated using the content of all the message and allow to verify the integrity of the entire datas.

Ack and Ack enable
------------------

**Ack** is a small byte sent by the receiver after a message to say "ok Roger, received 5/5". This feature can be enabled or disabled for each message using the **Ack_enable** flag.
After the end of the message (after the **CRC** bytes) the receiver have a **Ttimeout** to send back his **Ack**. From the other side the transmitter wait a **Ack** during **Ttimeout**, if **Ack_enabled** is anabled and no **Ack** has been receive during the **Ttimeout** period the message will be considered as failed.

/!\ This features can't be used for multicast or broadcast messages.

Discoverring sequence
---------------------

The **discovering sequence** is used to detect the topology of the network and distribute IDs to each modules. This sequence is started by a [**gate module**](modules_types.md).
The **gate module** who start this sequence try to discover a neigbour using the **PTP** line, if a module receive the signal he lock the **PTP* as reply and wait it's **UniqueID** and eventualy it's **VirtualID** from the gate.
The gate detect the lock sent by the module and start to distribute Ids. When the first module have all it's ID he try to detect a downstream neighbour using the **PTP** line.
This routine continue until a slave don't detect any neighbour on the downstream side. When a "end of a line" is discovered by the last module he unlock the upstream **PTP** line, then the previous module do the same things and so on.
When everything is detected the gate receive the unlock of his **PTP** line.
