
![babbar,gif](https://github.com/k-five/bline/blob/master/screenshot/banner.gif)

<br>


█░░░█ █▀▀ █░░ █▀▀ █▀▀█ █▀▄▀█ █▀▀  
█▄█▄█ █▀▀ █░░ █░░ █░░█ █░▀░█ █▀▀  
░▀░▀░ ▀▀▀ ▀▀▀ ▀▀▀ ▀▀▀▀ ▀░░░▀ ▀▀▀  
to "beautiful line output" repository.  
A simple CLI application for colorizing standard output written in `C`.  

This is an application that you can use on your Console/Terminal.  
It provides you some simple + friendly + flexible options that you  
can use easily.  

There is no prerequisite except **compile** it with a `C` compiler.  

Core features:  
 - support 3/4 bits : `ESC[Value;...;m`  
 - support 24 bits  : `ESC[(38|48);2;...;m`  
 - code generation (= dump code)  

### how to use:  

 1. `git clone https://github.com/k-five/bline`  
 2. `cd src/`  
 3. `make build`

### how to use (screenshot):  

![how-to-use.png](https://github.com/k-five/bline/blob/master/screenshot/how-to-use.png)  

  
Then you have a binary file named: **bline** and you can put it in a valid path  
that you have like: `/usr/bin/` or create your own path in `~/bin/` an put it  
there. Or just use it in-place: `ls | ./bline -a red`    
  

### example:  
```
# all lines, foreground red:
ls | bline -a red

# all lines, foreground red, background green:
ls | bline -a red:green

# all lines, foreground is default (NOTE.3), background is green:
ls | bline -a :green

# add color-mode, odd lines (foreground) will be in light,italic,red
ls | bline -o light:red

# add text-mode, as well
ls | bline -o light:italic:cross:red

# combine options are okay:
ls | bline -a light:green -i red -I 7 -r yellow -R 5-10

# using hex value, foreground in red
df | bline -H -a ff00ff

# hex, foreground green, background red
df | bline -H -a 00ff00:ff0000

# hex just background
df | bline -H -a :ffff00
```

### NOTE(s)  
 1. The app creates code dynamically. If you are interested in see: `ansi_code.h`   
 2. For non-HEX value it supports **color-mode** and **text-mode** and **foreground-color** and **background-color**.  
 3. But for HEX value it supports only **foreground-color** and **background-color**.  


### Your Feedback  
Let me know if you have/had any issue(s).  
You can raise an issue or contact me via the email on the page of my profile.  

### Goal
This app have been designed mostly for educational purpose then colorizing some lines.
Although it looks like that, but I added a lot of comments so you can read the source
code easily. The two main goals in this app are:
 1. how to use pointers and pointer arithmetic effectively
 2. how to use heap effectively  

### code generation example (screenshot)  

![dump.png](https://github.com/k-five/bline/blob/master/screenshot/dump.png)  

For more screenshots see **screenshots** directory


<br>


### More details on  
 - [Wikipedia](https://en.wikipedia.org/wiki/ANSI_escape_code)  
 - [How to change the output color of echo in Linux](https://stackoverflow.com/questions/5947742/how-to-change-the-output-color-of-echo-in-linux/28938235#28938235)  
 

<br>


### License  
License GPL-3  
