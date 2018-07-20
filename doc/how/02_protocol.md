## What is on Robus messages

All Robus message have this structure : 

>**Preamble - Header - Data[N] - CRC - ACK**

### Preamble
The Preamble is used to detect the possibility of a new message reception or transmission. This is a mandatory inactivity period of time on the network between each messages. This period of time is equivalent to the timeout period, so if a module is stucked on a message reception the preamble will reset his state to receive a new message. The duration of the preamble (and timeout) can be configured at (N x byte time) scale.

### Header
The header a 6 bytes field containing all informations allowing a module to understand a message. All modules on the network catch it and decode it.
>**Protocol - Target - Target_Mode - Source - CMD - Size**

- **Protocol (4 bits)**: This field give the protocol revision. If in the futur we change the **Header** content this **Protocol** field indicate to the module how to understand it or drop it if it can't read this protocol version.
- **Target (12 bits)**: This field contain the target address. Watch out to understand the real destination of this field, you have to know the addressing mode contained on the **Target_mode** field.
- **Target_mode (4 bits)**: This field indicate the addressing mode and how to understand the **Target** field. It can take different values :
  - **ID**: This mode allow to say something to an unic module using his ID whithout acknoledgement return
  - **ID+ACK**: This mode allow to say something to an unic module using his ID whith acknoledgement return
  - **Multicast/Broadcast**: This mode allow multiple module to catch this message. In this case the message contain a type of data used by multiple modules.
  - **Type**: This mode send a message to all modules whith a given type, for example all "Sharp digital distance sensor".
- **Source (12 bits)**: The unic ID of the transmiter module.
- **CMD (8 bits)**: The command define the type of the trasmitted data.
- **Size (8 bits)**: Size of the incomming data.

 ### Data[N]
The main data of the message can be 255 bytes max

### CRC
The CRC allow to check the integrity of the received message (**Header** + **Data**)

### ACK
The acknoledgement is an optional field determined on the **Header** by the **Target_mode** field. The **ACK** is a 1 byte bata containing the status of the receiver send from the receiver to the transmitter (the invers direction of the previous fields).