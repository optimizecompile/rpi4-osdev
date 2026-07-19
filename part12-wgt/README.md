Writing a "bare metal" operating system for Raspberry Pi 4 (Part 12)
====================================================================

[< Go back to part11-breakout-smp](../part11-breakout-smp)

Porting the WordUp Graphics Toolkit
-----------------------------------
Back in the mid-1990s (when I was young!), programmers who wanted to build their own games didn't have rich frameworks like Unity. Perhaps the closest we got was the WordUp Graphics Toolkit, which I came across on the Hot Sound & Vision CD-ROM - a BBS archive. If you have a moment, perhaps use Google to see what "bulletin board systems" were... nostaglia awaits!

Much like my very simple _fb.c_, the WGT provides a library of graphics routines which can be depended upon for reuse. This library, however, is much more fully-fledged than mine, and makes it easy to build sprite-based games (like Breakout, Space Invaders, Pacman etc.).

The directory structure
-----------------------
As I port the WGT to my OS (a.k.a. make it work on my OS), I am using the following directories:

 * _bin/_ : for WGT binary files (fonts, sprites, bitmaps etc.)
 * _controller-ios/_ : a sample Swift BLE controller for the iOS platform
 * _controller-node/_ : a sample Node.js BLE controller 
 * _include/_ : now contains _wgt.h_ and _wgtspr.h_ too (header files necessary for WGT code)
 * _samples/_ : sample "kernels" for my OS which exercise certain WGT library functions. To build them, copy one of these (and only one at a time) to the same directory as the _Makefile_.
 * _wgt/_ : the library itself. Where possible, I have stayed true to the original code, but do bear in mind it was written for the x86 architecture and we're on AArch64!

Please note: I am neither a Node.js developer, nor a Swift developer, and so the controllers are purely samples that serve my purpose. They are not intended to be exemplars! I am very aware of the multitudinous problems with both...

Building
--------
So... to build the first WGT sample simply type `cp samples/wgt01.c .` from the top-level directory, and then type `make`. When you boot with the generated _kernel8.img_ you will see the screen go into 320x200 (VGA!) mode and draw a white line from corner to corner. If you do, the library is doing its stuff!

boot/boot.S changes
-------------------
We're still booting into a multicore environment (just in case we need it), using the official spin-table boot from part10: only core 0 arrives at `_start`, while the firmware parks cores 1-3 until _lib/multicore.c_ releases them into `secondary_entry`. On top of that, this part makes a few significant changes to _boot/boot.S_:

 * Enable FPU (floating-point unit) access so we can do non-integer mathematics. And not just on the main core: the WGT library and the Bluetooth code both use floating-point, and the trap registers involved are per-core, so every core sets this up for itself
 * Switch from EL2 (hypervisor exception level) down to EL1 (kernel exception level), keeping the MMU disabled all the same. Since the firmware now starts us at EL2 rather than EL3, the old `scr_el3`/`spsr_el3`/`elr_el3` dance becomes `hcr_el2`/`spsr_el2`/`elr_el2` - the same idea, one level down. It lives in a little `el2_to_el1` routine that every core passes through on its way in
 * Set `cnthctl_el2` so that EL1 may read the physical counter. Without this, the very first `wait_msec()` would trap to EL2 - where we have no exception handlers - and hang the core. We never needed it when arriving from EL3, because that gate simply didn't apply
 * Implement a `get_el` function to check which exception level we're at (for debug mainly)

Some things from this part's earlier, `kernel_old=1`-era _boot.S_ are conspicuously gone, too. There's no timer setup any more - the firmware's stub has already programmed it, and `cntfrq_el0` is only writable from EL3, which is now above our pay grade. And the `spin_cpu` variables no longer need pinning to fixed addresses with `.org` directives to keep them clear of the growing boot code: nothing refers to them by address any more, so they're ordinary labels that the linker places wherever it likes.

Using the Node.js BLE controller (macOS)
----------------------------------------
The controller in _controller-node/_ turns your Mac into the mouse: it captures global mouse input with [uiohook-napi](https://github.com/SnosMe/uiohook-napi) and streams it to the Pi over BLE. Before you first run it:

```bash
cd controller-node
npm install
./rebuild-uiohook.sh
node main.js
```

That third step matters! The stock uiohook-napi has a nasty macOS bug when used from plain Node.js: every physical mouse click (and key press) arrives with a companion "system-defined" event, and the library parses it by synchronously handing work to the process's _main thread dispatch queue_. Frameworks like Electron drain that queue; plain Node.js never does. The result is that the hook thread deadlocks on your **first click** - mouse moves stream happily until you press a button, then everything goes silent while the process looks perfectly healthy. We found it by sampling the frozen process and catching the thread stuck in `dispatch_sync` waiting on a queue with no consumer!

_rebuild-uiohook.sh_ rebuilds the library's native module with _uiohook-darwin-event-tap.patch_ applied, which reads those event fields without the main thread's help (plus makes the event tap listen-only and teaches it to recover if macOS ever disables it). Re-run the script any time you `npm install`, as that restores the stock (broken) prebuilt binary. You'll need git and the Xcode command line tools installed.

One more macOS thing: the first run will prompt you to grant your terminal Accessibility/Input Monitoring permission (System Settings → Privacy & Security) - global mouse capture doesn't work without it.

Using the iOS BLE controller
----------------------------
To use the iOS BLE controller instead of the Node.JS controller, ensure that you have:

```c
#define IOS_CONTROL
```

at the top of each of _wgt/mouse.c_ and _lib/bt.c_. Without this `#define`, the code will be looking for the Node.JS controller (so remove these lines if that's what you want!).

Work in progress!
-----------------
There's always more that can be done, but I do think this was a good exercise in exploring the joy of getting other people's code to run on your own OS! It's quite a thrill.

_Do have a go at building some of the samples (hint: wgt20 and wgt60 are super fun!)..._

I'm going to move on from here now so we can continue to make progress on the OS itself.

[Go to part13-interrupts >](../part13-interrupts)
