# ddbus
IPC through a P2P network

ddbus is a P2P extension of the famous IPC DBus. 
It relies on a P2P network created using [pcat](https://github.com/roule-marcel/pcat)

ddbus talks and listen to cross-host "channels":
* Characters provided to stdin are broadcated to both local (using dbus) and remote (using pcat) ddbus instances that use the same channel
* Incoming characters from the channel are fed to stdout
* If providing stdin with a message of the form "+ip:port", it is interpreted as "Add peer 'ip:port' to the P2P network". Therefore, if some remote machine ip:port was listening to pcat connections (e.g. via `pcat <port>`), it will receive all messages from the channel

# Examples

## Local communication

You can make two processes communicate by typing
In 1st terminal
````
ddbus channel
````
In 2nd terminal
````
ddbus channel
````
Any text entered in one terminal will then display on the other one. Input and output of ddbus can easily be redirected (e.g., in shell scripts)

## Remote communication

Assume you have 3 machines whose IP addresses are 192.168.0.1 and 192.168.0.2 and 192.168.0.3
To make them communicate through ddbus, you just have to type:
On 192.168.0.1 terminal:
````
ddbus channel
+192.168.0.2
````
On 192.168.0.2 terminal:
````
ddbus channel
+192.168.0.3
````
On 192.168.0.3 terminal:
````
ddbus channel
````
Any text entered in one terminal will then display on the other one, exactly as is the local case. Thanks to pcat, this creates a fully connected P2P network between the 3 hosts. This way, even if we turn off 192.168.0.2, 192.168.0.3 is still able to communicate with 192.168.0.1. Moreover, when turning back on 192.168.0.2, the 2 other hosts will automatically reconnect to it
