# Elm CAN player

This is cross-platform tool for transmitting custom CAN packets via elm327 compatible device.

Features:
- play periodic messages
- play CAN dump from file. Each packet should be on separate line and corresponding to format: `<canID> <byte0> <byte1>...<byte7>`. 
  **All values should be hexadecimal**
  Example:
  ```
  048   00 00 00 00 07 00 E0 00
  109   00 00 10 00 00 00 01 00
  ```

Supported protocols:
- **HS-CAN** (ISO 15765, 11-bit Tx, 500kbps, DLC=8)
- **MS-CAN** (ISO 15765, 11-bit Tx, 125kbps, DLC=8)
