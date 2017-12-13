# NotAnOverlay

In video game hacking, being able to write and draw on top of the game’s frames allows uncountable ways to get new advantages, especially for ESP cheat (Extra Sensorial Perception) in which you can draw coloured boxes around players with their names, distance, weapons, health points and more, giving you an almost perfect situational awareness.

External cheats (external meaning that they run in a separate process) often use a straightforward system: creating a completely transparent window, without borders or menus, that stay always on top of others, with the same size as the game and at the same position.
This way one can simply draw whatever in this transparent window and for the user it will appear as if it is draw on top of the game’s frames themselves.
Here is how it’s done:

![External ESP overlay system flowchart](http://en.ciholas.fr/wp-content/uploads/sites/2/2017/12/External-ESP-common-design-flowchart-small.png)

Whether the overlays are detected or not has been an on-going controversial topic for a long time.
In my opinion overlays are detected, but one need to understand something, having one detected element in a cheat doesn’t mean that the whole cheat will be detected.
Imagine that the anti-cheat finds a window, same size and position of the game, transparent, always on top, etc… that’s suspicious right? But okay, some unknown application might do that, banning people on that only basis might lead to false positives and ruin the reputation of both the anti-cheat and the game.
Now let’s say that in addition to this suspicious window, the anti-cheat takes a look at the process owning this window with a simple call of GetWindowThreadProcessId and by taking a look at the system handles of this process, they notice that it has a process handle to the game with full access, despite the fact that the anti-cheat driver has callbacks that prevent this from happening.
Now it is beyond a simple oddity.
They could even make sure automatically that the handle is being used for cheating purposes by analysing what this process does, for example monitoring if it is running calls to ReadProcessMemory using the handle to the game frenetically, for example by placing hooks or with other methods.

Anyway, whatever is your opinion on this, I think we can all agree that doing without such a suspicious overlay window would be a good thing.
There has been interesting approaches, like for example leveraging applications whitelisted by anti-cheats having the approval to draw on top of game frames, such as Discord, FRAPS, TeamSpeak, NVIDIA, and many other gaming related software.
Another interesting approach is to use Direct Kernel Object Manipulation (DKOM) to remove all tracks of that window, making it impossible to find with common API functions.
I have started an Overwolf plugin but haven’t finished and I do not know if I will work again on that, but if I do, I’ll share that.

I thought a bit about other possible solutions to do without an overlay window.
An idea that come to most people’s mind is to do a 2D radar to locate enemies around, but let’s admit it: Once you have tasted a state of the art ESP with overlay, there’s not going back, I wouldn’t settle for less.
So I was looking for a way to have an overlay, but without overlay window.
Then I had an idea: Duplicating the game’s image in a window (classic, custom size, non-transparent, non-always-on-top, etc… to be undetectable because like any other window) and drawing the cheat overlay on top of that image.
I wanted it to be (1) efficient, it has to run smoothly, and (2) compatible with the classic overlay systems (some people use DirectX, others use OpenGL, GDI, and probably many others).
One way to allow this would be to have the frame of the game being the background of the window, then the rendered image of the cheat overlay would just be added on top of that.
To do that I used GDI functions, the main ones being GetDC, CreateCompatibleDC, CreateCompatibleBitmap, BitBlt, and StretchBlt.
I read several times that GDI was not very performent, so I was expecting something laggy, but in fact it runs absolutely smooth!
With DayZ Standalone it ran at about 160 FPS constant as you can see in the following video:

[![NotAnOverlay PoC live demo](https://img.youtube.com/vi/Jgcz5QRaQkI/0.jpg)](https://www.youtube.com/watch?v=Jgcz5QRaQkI)

It actually runs so smoothly that one can play by watching only the duplicated frame in the overlay window without problem.
I do not know if I will use this, but at least I know that this is definitely a viable solution.
