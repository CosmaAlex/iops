# Reverse Engineering challenge
Website: https://pwnthem0le.polito.it/recruitment/

-- Writeup by *iops* --

## Downloading the program
Download the program from the website under the section "REV" and unzip it in any folder. The content of the zip file is as follows:

```
.
+-- README.txt
+-- revme
```

*README.txt* contains information about what to do in this basic CTF example and gives some hints about how to tackle the problem.
In my case, the most useful piece of information I read in this file is the flag format : **ptm{something here}**

*revme* is an 64-bit, dynamically linked, ELF file (Executable and Linkable Format). This information can be retrieved using the `file` command on Linux.
This file is where the bello is.

## First look
Running `./revme` without arguments outputs `Usage ./revme key`.

Running the program with an argument gives different results. Examples:
```bash
$ ./revme ciao
> Wrong charachter c in input!
$ ./revme abc
> Where the duck u going?
$ ./revme pwnthem0le
> Wrong charachter p in input!
$ ./revme sannicola
> Where the duck u going?
$ ./revme a b c
> Usage ./revme key
```

This could be a hint that there are chars that are allowed to be entered, and others that are not. We will see this later.
By adding more parameters, the program prints the same information message given for 0 arguments (see above).

## Reversing
### Basic methods

Running `strings revme` spits out the human-readable strings contained in the file.
The result is a long list of strings, the majority of which are internally used by the executable to run properly. Nevertheless some lines pop out:

```
u+UH
[]A\A]A^A_
Usage ./revme key
Wrong charachter %c in input!
Where the fuck u going?
:*3$"
biglabbiglabicbigczzpcmigzxqmnbyglmglababbdhbahlazqxllhaabyabmasrydbagzbbmatbiglalzbipbvibbzcpgigcgvbiiclymglmlqmnqqlealamaaeabibigabiglabbiglab
```

We already saw some of these strings while trying to run the program blindly in the "First Look" section.
There is also a pretty long string that looks meaningless but contains "biglab", so it may be worth some attention for the reversing process.

Trying to input each of these strings as the first argument of the program does not give any useful result. We shall proceed with the big guns.
