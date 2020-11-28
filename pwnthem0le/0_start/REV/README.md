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

### Radare 2
Let's start reversing with Radare 2 tool. Run in the terminal `r2 revme` to get the radare 2 interactive shell.
To run a basic analysis use the `aaa` directive and, once completed, print the name and addresses of the functions succesfully recognized using `afl` directive. 
The result in my case was:
```
[0x00001100]> afl
0x00001100    1 46           entry0
0x000010a0    1 11           sym.imp.puts
0x000010b0    1 11           sym.imp.strlen
0x000010c0    1 11           sym.imp.printf
0x000010d0    1 11           sym.imp.srand
0x000010e0    1 11           sym.imp.exit
0x000010f0    1 11           sym.imp.rand
0x0000132b   21 465          main
0x000011e0    5 137  -> 60   entry.init0
0x000011a0    5 57   -> 54   entry.fini0
0x00001130    4 41   -> 34   fcn.00001130
0x000011e9    4 36           fcn.000011e9
0x0000120d    3 59           fcn.0000120d
0x00001248    7 227          fcn.00001248
0x00001000    3 27           fcn.00001000
[0x00001100]>
```
The most eye catching function is the `main` function which is stored in offset `0x0000132b`. So we can put our offset in the file exactly at the beginning of this function
using the `s` command, in order to print a rough disassembled code with `pdf` command:
```
[0x00001100]> s 0x132b
[0x0000132b]> pdf
            ; DATA XREF from entry0 @ 0x1121
┌ 465: int main (int argc, char **argv);
│           ; var char **var_30h @ rbp-0x30
│           ; var int64_t var_24h @ rbp-0x24
│           ; var char var_1ah @ rbp-0x1a
│           ; var int64_t var_19h @ rbp-0x19
.
.  A very very long code
.
│      └──< 0x000014ef      0f8c88feffff   jl 0x137d
│           0x000014f5      b800000000     mov eax, 0
│           0x000014fa      c9             leave
└           0x000014fb      c3             ret
[0x0000132b]>
```
It is easy to tell that this is not the clearest of the representations for the code. However we can use the graph mode in radare2 to simplify the process of understanding the
control flow. To do so simply write `VV` command in the interactive shell. The result is the following view:
![Graph screenshot](https://github.com/CosmaAlex/iops/blob/main/pwnthem0le/0_start/REV/images/graph1.png?raw=true)

