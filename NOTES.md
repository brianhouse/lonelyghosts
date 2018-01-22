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


ok, this feels like they are syncing better. limited neighbors.

ok, 63 are now convincing.

now we're at 72. but it feels like it's falling apart.

listing 57 now.

but see, I'm not totally convinced everyone is registering.
ok, just counted, they basically are.


put bump amount / sensitivity at 0.02
maybe made a difference?


it does seem to be true that the ones that stray are not as closely coupled

//

so how much of this has to do with the pi itself? seems to have a bigger effect than I thought.

so I should try this from Granu.

synchub is using like 10% cpu and 1.1 memory. so it's not really that, though who is to say with network performance stuff.



ok, I kind of got the impression that it would hit a tipping point of sync, and then go all batty again.
so if there are max 10 neighbors, maybe should be no more than 0.01 each ... no wait that makes no sense

wait, but wow. it's a lot more stable with a lower bump (0.008)

this is 52 and right good.

62 worked also.

72 is rough.

putting bump at (0.004)

hmm. 


I wonder if what we're seeing now is this graph where two sides of the graph could be pulling in opposite ways.

I mean, thankfully, the network seems to hold for 72 when the neighbor size is limited.

putting it back to 0.008
maybe worse actually.
back to 0.005

and then I'm going to neighbor it at 15.



//

so you could actually separate the network. make sure there are no crossovers, with a max neighborhood size of 10, or whatever.

use a nearest neighbor kind of situation.

this is assuming the cross is the issue.


//

it seems like they were propagating based on proximity. which is really nice. is it true?
the neighbors are stored as sets, which are not ordered. but, I mean, they _are_ ordered somehow.
wait, but it was literally spreading right to left.
well, maybe it's those intermediate nodes.

it was beautiful though. was that with 50 some nodes?


/

have 10 buckets.
new scan comes in ... assign it to the bucket with the most neighbors. 
if none, the bucket with the fewest members.

can members get bumped? youd kind of want them to.

so you have the neighbors set for each node, and then a score based on how many in that set are in the bucket.

scan comes in, remove that guy. recalculate all bucket scores for everybody, and the bucket score for the scanning guy, for each bucket.


or basically, recalculate everything for each scan that comes in.












