if you find a bug (which you would probabily would because its
an initial relese) or add a new feature, please open an
[issue](https://github.com/zohaib2k2/zzblue-cups/issues).


## What is zzblue-cups ##

zzblue-cups is a yet another simple server that that allow you 
to submit cups print jobs through bluetooth. This program is especially handy
when it comes to making leagacy printer wireless through using raspberry pi.


## Prerequisite and Dependencies

The program requires cups and bluez packages needs to be installed and configured.
bluetooth daemon in compatibility mode, [here](https://raspberrypi.stackexchange.com/questions/41776/failed-to-connect-to-sdp-server-on-ffffff000000-no-such-file-or-directory)
how you can do that 



## Compiling

As of right now the only way to install is through compiling the program

```
gcc zzblue-cups-sv/zzblue-cups-sv.c -lbluetooth -lcups -g -o blue-cups-sv  
```

and you can run the program by running 

```
./blue-cups-sv
``` 

##Configration

see config.h and after editing the file recompile the program.

##Sponsoring

Fixing bugs and adding new features takes a lot a time and efforts 
in order to motivate the me to continue the maintenance of 
the software please consider making donation to
 [indus hospital](https://indushospital.org.pk/donate/)


