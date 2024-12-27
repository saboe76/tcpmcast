# tcpmcast

Connect to TCP and put every chunk received to a multicast group / port with additional data.

When a Message `MMMMMMMM` is received, an unix epoch timestamp with milisconds and a white space is prefixed. And if the last byte of the message is not a new line, then a new line is apended.


```
'MMMMMMMM'   ->   '1000000000.123 MMMMMMMM\n'
```

This tool is used for connecting to a PLC TCP mirror port, which is usually limited to only one connection, and broadcast the pimped telegram to as many consumers who are intressted in.
The time value in front of the telegram is important to calculate speed, time variation and jitter.
