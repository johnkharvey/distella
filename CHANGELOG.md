# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),

## [Unreleased]

 - General cleanup and more modern/github-friendly formatting

## [3.0.1b](https://github.com/johnkharvey/distella/releases/tag/3.01b) - August 26, 2016

### Bugs fixed

 - Crash when ABS_INDIRECT was encountered in some ROMs.
 - Labels were incorrectly wrapped 2600 style on 7800 ROMs, at 0x0fff.

### Original Release Announcement

 -  [https://atariage.com/forums/topic/255626-64-bit-assemberdisassembler-for-7800/page/2/?tab=comments#comment-3580588](https://atariage.com/forums/topic/255626-64-bit-assemberdisassembler-for-7800/page/2/?tab=comments#comment-3580588)

## [3.01a](https://github.com/johnkharvey/distella/releases/tag/3.01a) - August 14, 2016 release

### Bugs fixed

 - Crash in Linux/OSX when launching without a ROM file.
 - Compile errors in Linux/OSX from a stray illegal character in queue.c, and uppercase/lowercase filenames.
 - Crash under certain circumstances when processing RELATIVE addressing.

### New Features

 - This release includes builds for Linux, OSX and Windows, which should run on 32-bit and 64-bit machines.
 - Also included is a cross-platform Makefile (using gcc/make).

### Original Release Announcement

 - [https://atariage.com/forums/topic/255626-64-bit-assemberdisassembler-for-7800/?tab=comments#comment-3571430](https://atariage.com/forums/topic/255626-64-bit-assemberdisassembler-for-7800/?tab=comments#comment-3571430)

## [3.0](https://github.com/johnkharvey/distella/releases/tag/3.0) - April 22, 2003

### Bugs fixed

 - Changed VDEL01 equate to VDELP0 in vcs.h
 - Fixed indirect absolute indexing output to include proper spacing.

   Old method:

   `JMP($2105)` // Won't compile with DASM!

   New method:

   `JMP    $2105` // Will compile with DASM!

     Granted, this problem was generally invisible on the 2600, since without external hardware, there was no RAM higher than the zero page.  Although, note that the "Bachelor Party" ROM exhibits this problem (using `distella -pafs`, and no config file):
     
               STA    AUDV0   ;3
               JMP    (L3FFE) ;5

    This problem shows up in the following ROMs:
    
    1. Bachelor Party
    1. Mines of Minos
    1. Reactor

 - Fixed absolute,Y and ,Y indexing case for I/0 equates:

     - `STA SWCHA,X` or
     - `LDA SWCHA,X` or
     - `STA SWCHA,Y` or
     - `LDA SWCHA,Y`

    for any equated value numbers between 0x0280-0x0297 (so, not just limited to SWCHA).
    
    These would incorrectly disassemble, and drop the ,X or ,Y indexing.  The "Indy 500" ROM is the only known ROM that exhibits this problem (using `distella -pafs`, and no config file):
 
        LF006: STA    VSYNC,X ;4 // fixed
               STA    SWCHA,X ;5 // no ",X" in distella 2.10.
               INX            ;2

 - Fixed indirect absolute addressing for I/O equates.

   Though in practice, you never want to do this:
   
               JMP (SWCHA)

   for any equated value numbers between `0x0280-0x0297`, the disassembly of this instruction returned values not contained in the equates index table.  So, this has been fixed.

   Granted, this situation will most likely never appear in any 2600 programs, but this case is now fixed to not dump random characters of garbage into the output file, and will successfully return the equated String.  No known ROMs exhibited this phenomenon.

 - Fixed a spacing issue that is a problem when the `'-s'` option is enabled.

   In any given line, the whitespace after an instruction is added, to make any given line in the file exactly 15 characters in length.

   This makes the cycles appear lined-up in a column.  What was done was printing 15 minus the number of characters used in the current line worth of spaces, to line things up.  But, some games had lines greater than 15 characters per line, which caused "-1" spaces to be printed out.
   
   Since this doesn't make sense from a computer standpoint, it would interpret that value as a very large positive number, and print a lot of spaces in a row to make up for it.  The condition has been corrected.
   
   Example: `ACTIONMN.BIN`
       
              BRK            ;7
              .byte $80 ;.NOOP;2  // fixed output.  Previous output
                                  // had many lines of spaces between the NOOP
                                  // and the ;2.

 - Simply recompiling the source code for Distella 2.0 got rid of a few problems.
   1. Air Raiders, Bogey Blaster, Mines of Minos, and Star Wars: Jedi Arena did not show the full amount of used TIA equates.
   1. Riddle of the Sphinx and Shootin' Gallery incorrectly display the "INTERRUPT:" start address, even though the interrupt flag is not set.

 - For some ROM files, if the code is doing something unconventional (like a `BRK` that never has a corresponding `RTI` (i.e. because of manual stack manipulation), then it's possible to accidentally start disassembling segments that contain data or graphics instead of code (this is covered in more detail at the end of README.txt, in the "Limitations" section).

  In these situations, it's possible that a range of addresses that reaches to the end of a file could be marked for disassembly.  In these cases, the bug was that this range of addresses would not be disassembled appropriately.  At most, the first and last instruction in the segment would be disassembled, missing all of the code in-between.  So, a fix is in place to handle this.
  
  ROM files affected by this are:
    1. Alien's Return
    1. Bachelorette Party
    1. Col 'N' (HomeVision)
    1. Diagnostic cartridge, serial # MA017600
    1. Homerun
    1. Knight on the Town
    1. Lady in Wading
    1. Lost Luggage

  Also, as a subset of this problem, the last byte (or 2) of the code in these situations will be disassembled as code.  Normally this is ok, but if these particular bytes are translated to multi-byte instructions ( like a `"BMI"`, for example), there is a problem.  In these instances, a second byte (and possibly a third) byte is created to complete the multi-byte instruction.

  For now, this byte (or bytes) is believed to be made up from a random number.  This unfortunately causes any recompiles to make the end file size become a byte or two greater than it was when it was disassembled.  It also can cause distella to think that another piece of code is referenced, and it could even try to disassemble code at that address (i.e. in a graphics data area)!

  This problem has the potential to affect games where the second-last byte is translated to a 3-byte instruction, or where the last byte is translated to a 2- or 3-byte instruction.

  Known ROMs affected:
       
    1. Bachelorette Party
    1. Homerun
    1. Knight on the Town
    1. Lady in Wading
    1. Lost Luggage

### New Features
#### Changes for Atari 7800 support:
 - added command line flag `-7` for Atari 7800 support (please read more information under "options" in README.txt)
 - Added command line flag `-k` for POKEY sound equates (7800 mode only, please read more under "options" in README.txt)
 - Added command line flag `-b`, and changed the definition of the `-i` flag (for more information, please read the "options" section in README.txt)
 - Optimized memory allocation to be dynamic instead of static for `mem[]` and `labels[]` arrays

### Original Release Announcement

 - [https://atariage.com/forums/topic/24981-distella-30-released/](https://atariage.com/forums/topic/24981-distella-30-released)

## [2.10](https://github.com/johnkharvey/distella/releases/tag/2.10) - February 25, 1997

### New Features

- Added a cycle-counting option, which should prove helpful not only in examining existing code, but debugging your own code.

### Original Release Annoumcement

 - [https://www.biglist.com/lists/stella/archives/199702/msg00016.html](https://www.biglist.com/lists/stella/archives/199702/msg00016.html)

### Historical Notes

 - The 2.10 development page can be found here:
  - [https://web.archive.org/web/20010607091214/http://members.home.com/rcolbert1/distella.htm](https://web.archive.org/web/20010607091214/http://members.home.com/rcolbert1/distella.htm)
 - The 2.10 original code zip file with executable and source code can be found here:
  - [https://web.archive.org/web/20010614162046/http://members.home.com/rcolbert1/zip/distella.zip](https://web.archive.org/web/20010614162046/http://members.home.com/rcolbert1/zip/distella.zip)

