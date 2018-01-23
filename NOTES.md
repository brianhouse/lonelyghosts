Notes
=====

the full battery lasted from 17:45 until 00:07. so call it 6 hours.


started charging at 9:35
still charging at 12:15
was charged at 19:20


//

remember, the router is set to assign the constant ip to synchub if it connects through wifi.
if it connects via ethernet, the static ip on the pi kicks in (same address)
if granu is connected to synchub via ethernet w/ manual ip, synchub wont be able to use the shared internet
synchub wifi is free to connect to external (studio/gallery) wifi, and granu can access it that way
but for synchub to access the internet via wifi, the ethernet has to be unplugged

best situation: synchub connected to the router by ethernet, wifi for access (but unplug ethernet for internet / github)

got it?

using ethernet cuts down on bandwidth usage:
unit -> router -> broadcast -> hub -> router -> broadcast -> unit
vs
unit -> router -> hub -> router -> broadcast -> unit


//

host: synchub
u: pi
p: lonelyghosts

//

"Many individual wireless routers and other access points support up to approximately 250 connected devices."

A Wi-Fi router rated at 300 Mbps with 100 devices connected, for example, can only offer on average 3 Mbps to each of them (300/100=3). (https://www.lifewire.com/how-many-devices-can-share-a-wifi-network-818298)

...which is fine. so this could work.

//


ok, 53 are believablely connected to the GL now. but syncing is a little challenging for them.

I think I need to restrict the number of neighbors.

wow totally. 63 are now convincing.

now we're at 72. but it feels like it's falling apart.

it does seem to be true that the ones that stray are not as closely coupled

so how much of this has to do with the pi performance itself? seems to have a bigger effect than I thought. synchub is using like 10% cpu and 1.1 memory. so probably not.

ok, I kind of have the impression that it hits a tipping point of sync, and then goes all batty again.
so if there are max 10 neighbors, maybe should be no more than 0.01 each in a bump ... no wait that makes no sense

wait, but wow. it's a lot more stable with a lower bump (0.008)

this is 52 and super tight.

62 works also.

72 is rough.

putting bump at (0.004)

hmm. 


I wonder if what we're seeing now is this graph where two sides of the graph could be pulling a node opposite ways.

amazingly, the network is holding for 72 when the neighbor size is limited.

putting it back to 0.008
maybe worse actually.
back to 0.005

//

so you could actually separate the network into subgraphs. make sure there are no crossovers, with a max neighborhood size of 10, or whatever. and then make sure theyre closely coupled within that.

but what the hell was that propagating effect, that was really nice. how would the beep in order of spatial proximity? maybe it is via joiner nodes.

regardless, tight coupling in distinct groups is the way to go I think if we're going to get this to 96.


//

50 is solid.

so at 70, theyre all connected, and it's totally synctastic. but the checkin number is slipping.

also, weirdly, they are ALL syncronized. and I'm wondering if it's the delay caused by printing the network.

interesting. disturbing them all actually kicked a bunch off the network. not great. too much traffic?

eliminating some log output and saving network files seemed to help. eliminating disr for max stability.

