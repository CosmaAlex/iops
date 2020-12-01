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
The most eye catching function is the **`main` function** which is stored in offset `0x0000132b`. So we can put our disassebler offset in the file exactly at the beginning of this function
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
It is easy to tell that this is not the clearest of the representations for the code. However we can use the **graph mode** in radare2 to simplify the process of understanding the
control flow. To do so simply write `VV` command in the interactive shell and press Enter. 

The result is the following view:


![Graph screenshot](https://github.com/CosmaAlex/iops/blob/main/pwnthem0le/0_start/REV/images/graph1.png?raw=true)

In an overview of the main function, there is some sort of loop on a string as you can tell from the call of `strlen` and the following conditional jump `jl 0x137d`, then there are some checks on the iterated string content with a trap if the content is not valid, then there is a mysterious part at 0x1419 doing some moves and shifts, then there is a further checking on the output of the mysterious code block with a trap and finally, if all the checking are ok, there is the call to a function `fcn.00001248` which definetly looks like a *`print_flag()` function*, to which a `char*` argument is passed. 

For this reason I renamed the function print_flag pressing the `:` button to enter command mode and typing the following command: `afn print_flag 0x1248` (0x1248 is the offset taken from the previous fcn name). Then refresh the graph view by pressing `r`.

### Analyzing print_flag function

As a good rule of thumb it is always a good idea to check immediately the `print_flag` function if there are hard coded constants or if the calculation are independent of the main function. To print the graphed dissasembled version of `print_flag` we will move the offset to print_flag beginning (0x1248) by typing in command mode `s 0x1248`. Then press Enter twice and the graph view will update with the new function.

The result is the following:

![Graph screenshot](https://github.com/CosmaAlex/iops/blob/main/pwnthem0le/0_start/REV/images/graph2.png?raw=true)

This function seems to be using the **`rand()` standard C function** which is a pseudo-random() number generator included in the C standard library. To generate numbers it needs a **seed**, and in the code you can see that radare conveniently recognized that there is a variable that works as a seed and appropriately named it `seed`. Here we can make an observation: after setting the seed via the `srand()` C function, the numbers generated by the following `rand()` function calls will be different but, if you call in some time in the future `srand()` with the same seed, the new numbers will be the same as the previous run because the generation algorithm is **pseudo-random**!

The `print_flag` function seems to first calculate the seed (which has to be of type `unsigned int` btw) according to the string passed as parameter and then, on the basis of the calculated seed it performs some funky arithmetic to calculate an output string named `s` by the disassembler, most probably, the flag.

The fact that **the parameter used in the input is just transformed into a number** to be used as a seed for a pseudo-random number generator is a great thing! This is because we can brute-force the `seed` variable and perform the flag calculation algorithm for each seed very quickly until we find the almighty flag.

### Brute-forcing

To perform this computation you can use `ghidra` to disassemble the code and get a more readable C version. After opening the executable in ghidra and analyzing it there is in the Symbol Tree window, in the Functions section, a function at the same offset that we got in radare2 which was 0x1248. Conveniently enough the function that is interesting to us has the offset in the name: `FUN_00101248`, and it looks like this:

![Graph screenshot](https://github.com/CosmaAlex/iops/blob/main/pwnthem0le/0_start/REV/images/ghidra1.png?raw=true)

To be more specific, the flag computation happens from the `srand()` call. So you can copy and paste that part into a new `main.c` file. After some changes to make the code more readable it becomes:

``` C
srand(seed);
for (i=0; i < (int) strlen(tmp_key); i++) {
	int rand_val = rand();
	tmp_key[i] = tmp_key[i] ^ (char) rand_val + (char)(rand_val/0xff);
}
```

The variable `tmp_key` refers to an hard-coded constant in the function that is called `DAT_00104020` in the ghidra project. Its content can be accessed by double clicking on it and looking at the Listing window. At this point everything is ready and we can try different values for the `seed` variable. Our termination condition will be when we find a string beginning with `ptm{` as we expect from reading the `README.txt` file.

The complete program to test all the seed numbers is in the repo (`main.c`). After running it we get:

```
The seed required was 3~~0
The decrypted flag is ptm{y0u_~~~~~~~~~~~~~~~_l4b?}

n i c e

```

And there we go, we got it!


### My Learning Observations

Look at `print_flag()` function very closely before switching to `main()`.