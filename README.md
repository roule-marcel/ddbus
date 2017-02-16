# ddbus
IPC through a P2P network

ddbus is a P2P extension of the famous IPC DBus. 
It relies on a P2P network created using [pcat](https://github.com/roule-marcel/pcat)

ddbus talks and listen to cross-host "channels":
* Characters provided to stdin are broadcated to both local (using dbus) and remote (using pcat) ddbus instances that use the same channel
* Incoming characters from the channel is fed to stdout
* If providing stdin with a message of the form "+ip:port", it is interpreted as "Add peer 'ip:port' to the P2P network". Therefore, if some remote machine ip:port was listening to pcat connections (e.g. via `pcat <port>`), it will receive all messages from the channel



