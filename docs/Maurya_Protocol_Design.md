
* Client => Server: Client Allocation Request
  * 0x45BD (from YR1.001 EIP)

* Server => Client: Client Allocation Response
  * 0x45BD
  * u16 Client Session ID
  * 32B Client Session Key

The allocation guanxi is then used as Rejuvenation guanxi.

* Client => Server: Rejuvenation Request
  * 0x424A
  * u16 Rejuvenation Sequence Number

* Server => Client: Rejuvenation Response
  * 0x424A
  * u16 Rejuvenation Sequence Number

* Client => Server / Server => Client: Goodbye
  * 0x4C44

* Client => Server: Guanxi Allocation Request
  * 0x0061
  * u16 Client Session ID
  * 32B Client Session Key
  * SOCKS5 Address

* Server => Client: Guanxi Allocation Response
  * 0x0061
  * u8 Gossip Code (SOCKS5)
  * u32 Guanxi ID

* Client => Server / Server => Client: Data Packet
  * 0x4145
  * u8 FLAGS
    * Tunneling
    * Ping-Pong
    * Purge
    * Gossip
  * u32 Guanxi ID
  * u64 Policy Sequence Number
  * u64 Highlight Sequence Number

  If Tunneling is on:
  * u16 Length
  * Data

  If Ping-Pong is on, Tunneling can't be on:
  * u64 Ping-Pong Sequence Number
  * 32B Client Session Key (Client => Server Only)

* Client => Server: Ping-Pong Network for Guanxi Request

* Server => Client: Ping-Pong Network for Guanxi Response
  Tunneling Packet

---

Not really useful:

* Client => Server: Extra Heartbeat Guanxi Request
  * 0x441C
  * u16 Client ID
  * 32B Client Key

* Server => Client: Extra Heartbeat Guanxi Response
  * 0x441C
