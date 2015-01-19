doom3do
=======

The complete archive for DOOM for the 3DO
=========================================

Yes, this is the infamous port of DOOM for the 3DO. Firstly, this
was the product of ten intense weeks of work due to the fact that
I was misled about the state of the port when I was offered
the project. I was told that there was a version in existance
with new levels, weapons and features and it only needed "polishing"
and optimization to hit the market. After numerous requests
for this version, I found out that there was no such thing and
that Art Data Interactive was under the false impression that
all anyone needed to do to port a game from one platform to
another was just to compile the code and adding weapons
was as simple as dropping in the art.

Uh... No...
v
My friends at 3DO were begging for DOOM to be on their platform and with 
christmas 1995 coming soon (I took this job in August of 1995, with a
mid October golden master date), I literally lived in my office, only taking
breaks to take a nap and got this port completed.

Shortcuts made...
=================

I had no time to port the music driver, so I had a band that Art Data hired
to redo the music so all I needed to do is call a streaming audio function
to play the music. This turned out to be an excellent call because while
the graphics were lackluster, the music got rave reviews.

3DO's operating system was designed around running an app and purging, there
was numerous bugs caused by memory leaks. So when I wanted to load the Logicware
and id software logos on startup, the 3DO leaked the memory so to
solve that, I created two apps, one to draw the 3do logo and the other to
show the logicware logo. After they executed, they were purged from memory
and the main game could run without loss of memory.

There was a Electronic Arts logo movie in the data, because there was a time that
EA was going to be distributing the game, however the deal fell through.

The vertical walls were drawn with strips using the cell engine. However, the 
cell engine can't handle 3D perspective so the floors and ceilings were drawn
with software rendering. I simply ran out of time to translate the code
to use the cell engine because the implementation I had caused texture tearing.

I had to write my own string.h ANSI C library because the one 3DO supplied
with their compiler had bugs! string.h??? How can you screw that up!?!?! They
did! I spent a day writing all of the functions I needed in ARM 6 assembly.

This game used Burgerlib 2. My first "C" version of Burgerlib because Burgerlib
was originally written in 65816 for the SNES and the Apple IIgs. If you
check out Burgerlib 5 (The current version, also on github), you'd notice
that some code is still in use.

I hope that everyone who looks at this code, learns something from it, and
I'd be happy to answer questions about the hell I went through to make this
game. I only wished I had more time to actually polish this back in 1995 so
instead of being the worst port of DOOM, it would have been the best one.

And one more thing...
=====================

The intellectual property of DOOM is the exclusive property of ZeniMax. 
No transfer of the intellectual property of DOOM or any transfer of the
ownership of the sounds, art or other game assets are given nor implied.
If anyone wishes to release a version of DOOM 3DO commercially, contact
ZeniMax for a license.

The source code... Go for it.

Rebecca Ann Heineman

Olde Skuul

Seattle, WA

