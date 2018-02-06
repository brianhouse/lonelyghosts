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


//

turning them all on simultaneously makes it take longer for them to all connect. 50 can do it. more, no.

checkins waver a bit. I think we're just dropping pings on the network.

the waves are totally still happening. so is the mass sync. insane. it's almost as if entrainment were a physical property of the universe.

80 is working, basically the same situation as 70.

90 is totally sync'ed. seeing all over 80s in the checkins. so not terrible.

95... not totally cohering. checkins jumping all over, down to 50.
limit is just the router.

reducing neighbors to 8. nada. back to 80 nodes? they sync.

interesting ... how long are the messaging timeouts? it's set to 200ms, but that's not how it's behaving.
should I timeout remove the node? no. it could come back. 

no sign of the bumping off the network thing.


should really have made a tool to ping to nodes or turn them off by id.


so, in conclusion. use ~80, unless the new router is able to handle more. otherwise, I think it is what it is.

gotchas? lots of people interacting, not taking time to sync, etc. 

have a docent that limits the number of people?


//

ok, today is a different day, I guess. 60 is great, solid.

now I have 80 that are behaving more or less synced.
but my checkins are bouncing like crazy. barely hitting 70.

restarted server. I'm getting checkins in the 70s now... 
dropping to high 60s

ok



so use 80. you should expect checkins in the 60s and 70s. 

I unplugged a box. not seeing timeouts, which is questionable.

hitting all the boxes.

clearly stronger sync. universal sync. seeing low 60s on the checkin.
ok, so this is actually totally consistent with my previous notes.

70 is the way to go. more starts to be unstable. 

/

switching to new router and restarting server
some problems getting it to stick

previously over 50 cannot simultaneously connect
60 connected like instantly here

checkins are a very stable 57
total sync

turning on another box caused total chaos.
flow of timeouts
not stable

so with the black router, it's 60. and that's the number.


//

back to mango 1

50 is the max for simultaneous connections, nothing connects when more are on
stable 48
adding a box
total synced with 60, but checkins wavering
adding a box

ok, something happened and the server dropped everything
recovered

now total sync with 70-
66 checkins showing

adding a box
totally connected and syncing with 80-
60s/70s checkins showing



ok. so I'm feeling confident now. this is how it behaves.
Black is sharper and more stable, but collapses higher than 60.
Mango is a little less stable, but can handle more. which could totally be a chip thing. so I ordered v2.


///////

you'll do 80 with the mango, and expect checkins in the 60s and 70s. maybe v2 gets better.
remeber not to try and connect more than 50 at a time.

neighbor_range: 40
max_neighbors: 10
bump_amount: 0.005


///////

thoughts -- total sync maximizes network burst, so there's probably less of an issue with this in the show. but still.



what am I worried about?
- the server getting corrupted, or doing weird shit. because that was weird.



/////

v2

all 50 connected immediately and are stable

added a box
60 are connected and stable


//

good firmware is 2.18
1B firmware is 2.25 -- unclear if it's the same


1B:
OpenWrt Chaos Calmer 15.05 / LuCI 15.05-109-g7bc4c48 Release (git-15.322.41240-4e44955)


////

granoff

trying to do two GL routers simultaneously, but they are conflicting or something -- cant do one on wifi and one tethered

it's because they have the same subnet

changing to 192.168.9.1