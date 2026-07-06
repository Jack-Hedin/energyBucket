# energy bucket

The goal of this project will be to simply make an analysis module that will proccess some events and but all the energies of each particle into an energy histogram


I need to figure out exactly where the data is coming from, so lets figure out where rtrk get the data, im gonna try to use the same data. if its a sim then im using that, if its actual data, then I will use that.

##rough abstract code v1
macro is going to input the same data as seen in the rtrk module, only the real data, dont know where the sim comes in yet, but then through the analysis module, all im going to do is have it preint the enrgy in each cluster, NOT Jet just yet, only a cluster. If I get confident, Im going to see if I can print the jets, then everything in the cluster.

##abstract
init Fun4All server -> input data files from list, (probably just the first one in the file but we shall see)
-> init analysis module, 
analysis module: 
find (outer) hcal node -> clusters -> print tranverse energy
