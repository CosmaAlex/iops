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
