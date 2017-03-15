## Robus communication optimisation ways (developpemental)

### Timestamp
To use timstamp in Robus we will need to synchronise all modules at the same date and update this date frequently.
This feature simplify debugging by giving a time notion. That allow to send big commande list to execute at a given date, to ask data from a given date, and in case of bus congestion that can invalid a too old command.

### Auto-speed
This feature could measure the SNR (Signal Noise Ratio) by measuring message CRC fail ratio and scale bus speed concequently.

### Alias management
A readable Alias is stored in the module memory, that allow to catch any modification into the hardware setup of the robot. The user can also use this Alias name instead of the ID thanks to routing tables.
