Simulating JellyFish attacks in NS3
=======

*This project is no longer being developed*
I attempted to implement JF attacks for a school project and was partially
successful, modules I created are being released in their unfinished state
to allow others to have a partial starting point if needed

I am not sure where the modified aodv module breaks down as it has been a
while scince I worked on this. It may be possible to insert delays by using
the scheduler (I did not try this).

Implementation files for JellyFish Attack in ns3 (v3.19) simulator.
Files (and directories) need to be placed with ns3 source files and recompiled
before simulations may use them

attacks by randomly dropping packets have been implemented,
attacks by packet reordering have been partially implemented (see below),
attacks by packet delay are unimplemented.

Packet reordering may fail as there is no garuntee that they will be pulled
out of the reorder window in the incorrect order.

Some basic simulations are provided in the scratch directory.
