# Project0-BMC
Students:
Francisco Chavarro Conde - 2019227569
Jose Pablo Fernández Cubillo - 2019047740

First of all you need to have installed Glade, in
order to compile the project, this can be done with
the command:

sudo apt install glade

Then there is a makefile which compiles the project.
It makes sure that all files and glade is added.

To run the program one goes to the command line and 
writes something like:

./mendel_crossing

Then a window will appear. The user can choose a file
that already exists or proceed to insert the number of
traits desired to cross the genotypes, the limit of
traits possible is 7, we tried with more but gtk comboboxes
could not handle it, or maybe it was my computer, still with
7 there is some waiting to be made when loading information
in the comboboxes. The user can also save the current traits
information in a txt file.