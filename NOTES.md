Notes
=====

the full battery lasted from 17:45 until 00:07. so call it 6 hours.


started charging at 9:35
still charging at 12:15
was charged at 19:20

//
remember, the router is set to assign the constant ip to synchub if it connects through wifi.
if it connects via ethernet, the static ip on the pi kicks in (same address)
if granu is connected via ethernet, synchub wont be able to use the shared internet
but synchub wifi is free to connect to external wifi for github et all
just connect granu via the project router to monitor

ok, no. setting that the same is causing the router to go away.


having problems with chrome losing the login to the router.

ok, so I think sketchiness was attached to having the router powered by the pi. keep it separate.
notice that the unit keeps pinging away, has no idea the server isnt there.


//

host: synchub
u: pi
p: lonelyghosts



//

ok, static ip is set for ethernet and I'm not getting a connection to the internet through external wifi. wtf?
it's pinging from the static address.

ok, I pull the plug, and it goes fine.

My router is 300Mbps.

Many individual wireless routers and other access points support up to approximately 250 connected devices. 

A Wi-Fi router rated at 300 Mbps with 100 devices connected, for example, can only offer on average 3 Mbps to each of them (300/100=3). (https://www.lifewire.com/how-many-devices-can-share-a-wifi-network-818298)

...which is fine. so this could work.

//

going to put the synchub wifi to studio wifi

and connect the router via ethernet:

unit -> broadcast -> router -> broadcast -> hub -> broadcast -> router -> broadcast -> unit
vs
unit -> broadcast -> router -> hub -> router -> broadcast -> unit

eliminates two broadcasts, so could have an effect.


unit -> router -> broadcast -> hub -> router -> broadcast -> unit
vs
unit -> router -> hub -> router -> broadcast -> unit

still takes out a broadcast. still worth it.

//


ok, 53 are believablely connected to the GL now. but syncing is a little challenging.

I think I need to restrict the number of neighbors.


//

exhibition checklist

make a SD clone
