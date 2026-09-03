Frame structure 

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R|opc1ode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16)              |
|N|V|V|V|       |S|             |                               |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+-------------------------------+
|                    Extended payload len C.                    | 
+-------------------------------+-------------------------------+
|    Extended payload len C.    |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+---------------------------------------------------------------+
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
|                     Payload Data continued ...                | 
+---------------------------------------------------------------+

# frame_serialization
opcode { Continuation(0x0), Text(0x1), Binary(0x2), Close(0x8), Ping(0x9), Pong(0xA) }

Basic_frame_structure {
  bool FIN = true;
  uint8_t opcode = 0x1; 
  bool MASK = false; 
  uint32_t mask_key = 0;
  vector<type_> payload;
} // default (CLI -> SER)

# Processing Step
```  [TCP connection] -> [TLS handshake] -> [Request WSS/HTTP upgrade] -> [Open websocket tunne] ```

# Framing Protocol
byte0.md
