
                               D i S t e l l a

                                   v 3.00

                                 Created By

                         Dan Boris and Bob Colbert

                               7800 Update by:

                               John K. Harvey

                                 Thanks to:

         Alex Hornby, Vesa-Matti Puro, Jarkko Sonninen, Jouko Valta

----------------------------------------------------------------------------

Quick Docs:  Type DiStella at the command prompt.


Differences from version 2.10:

 Bugs fixed (note that version 3.0 was tested on over 400 unique files):
   - Changed VDEL01 equate to VDELP0 in vcs.h

   - Fixed indirect absolute indexing output to include proper spacing.
         Old method:
           JMP($2105)   // Won't compile with DASM!
         New method:
           JMP    $2105 // Will compile with DASM!
       Granted, this problem was generally invisible on the 2600,
       since without external hardware, there was no RAM higher
       than the zero page.  Although, note that the Bachelor Party ROM
       exhibits this problem (using distella -pafs, and no config file):
         STA    AUDV0   ;3
         JMP    (L3FFE) ;5
       This problem shows up in the following ROMs:
         a) Bachelor Party
         b) Mines of Minos
         c) Reactor

   - Fixed absolute,Y and ,Y indexing case for I/0 equates:
         STA SWCHA,X or
         LDA SWCHA,X or
         STA SWCHA,Y or
         LDA SWCHA,Y
       for any equated value numbers between 0x0280-0x0297 (so, not
       just limited to SWCHA).  These would incorrectly disassemble, and
       drop the ,X or ,Y indexing.  The Indy 500 ROM is the only known
       ROM that exhibits this problem (using distella -pafs, and no
       config file):
          LF006: STA    VSYNC,X ;4 // fixed
                 STA    SWCHA,X ;5 // no ",X" in distella 2.10.
                 INX            ;2

   - Fixed indirect absolute addressing for I/O equates.
       Though in practice, you never want to do this:
         JMP (SWCHA)
       for any equated value numbers between 0x0280-0x0297,
       the disassembly of this instruction returned values not
       contained in the equates index table.  So, this has been fixed.
       Granted, this situation will most likely never appear in any
       2600 programs, but this case is now fixed to not dump
       random characters of garbage into the output file, and will
       successfully return the equated String.  No known ROMs exhibited
       this phenomenon.

   - Fixed a spacing issue that is a problem when the '-s' option is enabled.
       In any given line, the whitespace after an instruction is added, to
       make any given line in the file exactly 15 characters in length.
       This makes the cycles appear lined-up in a column.  What was done was
       printing 15 minus the number of characters used in the current line
       worth of spaces, to line things up.  But, some games had lines greater
       than 15 characters per line, which caused "-1" spaces to be printed out.
       Since this doesn't make sense from a computer standpoint, it would
       interpret that value as a very large positive number, and print a
       lot of spaces in a row to make up for it.  The condition has been
       corrected.
       Example: ACTIONMN.BIN
              BRK            ;7
              .byte $80 ;.NOOP;2  // fixed output.  Previous output
                                  // had many lines of spaces between the NOOP
                                  // and the ;2.

   - Simply recompiling the source code for Distella 2.0 got rid of a few problems.
     a) Air Raiders, Bogey Blaster, Mines of Minos, and Star Wars: Jedi Arena did
        not show the full amount of used TIA equates.
     b) Riddle of the Sphinx and Shootin' Gallery incorrectly display
        the "INTERRUPT:" start address, even though the interrupt flag
        is not set.

   - For some ROM files, if the code is doing something unconventional (like a BRK
       that never has a corresponding RTI (i.e. because of manual stack manipulation),
       then it's possible to accidentally start disassembling segments that contain data
       or graphics instead of code (this is covered in more detail at the end of this file,
       in the "Limitations" section).  In these situations, it's possible that a range of
       addresses that reaches to the end of a file could be marked for disassembly.  In
       these cases, the bug was that this range of addresses would not be disassembled
       appropriately.  At most, the first and last instruction in the segment would be
       disassembled, missing all of the code in-between.  So, a fix is in place to handle
       this.  ROM files affected by this are:
         a) Alien's Return
         b) Bachelorette Party
         c) Col 'N' (HomeVision)
         b) Diagnostic cartridge, serial # MA017600
         e) Homerun
         f) Knight on the Town
         g) Lady in Wading
         h) Lost Luggage
       Also, as a subset of this problem, the last byte (or 2) of the code in these
       situations will be disassembled as code.  Normally this is ok, but if these
       particular bytes are translated to multi-byte instructions ( like a "BMI",
       for example), there is a problem.  In these instances, a second byte (and
       possibly a third) byte is created to complete the multi-byte instruction.
       For now, this byte (or bytes) is believed to be made up from a random number.
       This unfortunately causes any recompiles to make the end file size become a
       byte or two greater than it was when it was disassembled.  It also can cause
       distella to think that another piece of code is referenced, and it could even
       try to disassemble code at that address (i.e. in a graphics data area)!

       This problem has the potential to affect games where the second-last byte is
       translated to a 3-byte instruction, or where the last byte is translated to
       a 2- or 3-byte instruction.

       Known ROMs affected:
         a) Bachelorette Party
         b) Homerun
         c) Knight on the Town
         d) Lady in Wading
         e) Lost Luggage


 Changes for Atari 7800 support:
   - added command line flag -7 for Atari 7800 support (please read
         more information under "options" below)
   - Added command line flag -k for POKEY sound equates (7800 mode
         only, please read more under "options" below)
   - Added command line flag -b, and changed the definition of the -i
         flag (for more information, please read the "options" section
         below)
   - Optimized memory allocation to be dynamic instead of static
         for mem[] and labels[] arrays




What is it?

    Distella is a disassembler specifically for the Atari 2600.  Since its
creation, it has been modified to disassemble Atari 7800 code as well. It
creates source code that is usually recompilable without any human
intervention. It examines the code and performs some basic tracing routines
which allow it to accurately distinguish data from code.

Features:

    o  Written in portable ANSI C - source code is included.
    o  Very fast
    o  Distinguishes data from code
    o  Uses labels for Atari 2600/7800 register locations
    o  Allows user to override or disable auto data determination
    o  Optionally includes 6502 cycle times as comments
    o  Freeware - Use it, Love it, Live it!

Command format:

    Distella [options] romimage [> sourcefile]

        Distella puts the sourcecode generated to standard output, so to
    put it in a file, use the '>' redirection.  Unlike Distella 1.0, the
    .bin suffix is not assumed, and you must use the -c flag to tell
    DiStella the name of a config file if you choose to use one.

Options:

  -7 -> Allows for MARIA equates and MARIA I/O equates tables to be loaded
	instead of the Atari 2600 VCS Equates and 2600 VCS I/O tables.
        Also, with this option, supported 7800 file sizes can be 16K, 32K,
        and 48K (or 128 bytes greater for each size, if a correct 7800
        header is appended at the front, which are a78 formatted files).
        Files larger than 48K are currently not supported, since they use
        bankswitching techniques (just like distella will not support
        Atari 2600 8K disassembly, since these files are bankswitched).
        To disassemble files larger than 48K, you will have to disassemble
        each bank (whether they be 16K or 32K) separately.  This is
        nontrivial, but instructions to do this can probably be found
        on the world wide web.

  -a -> Disables the printing of the letter 'A' for opcodes in which
        the A register is implied.  Some assemblers don't like the 'A'
        and others require it.

        Example:  LSL A  would be just LSL
    
  -b -> DiStella will read the address indicated in the last 2 bytes of
        the ROM file - the BRK vector - and trace through it to
        help determine data areas.  Not all programs use the BRK
        vector.  The best thing to do is try disassembling the image
        without this flag and see if the last two bytes point to an
        area that is not disassembled.  If that is the case, try the
        flag and see if the interrupt routine contains valid code.  It
        should be noted that a majority of games do not use the interrupt.
        This is true for both Atari 2600 games and Atari 7800 games.

  -c -> Defines a config file to use for disassembly.  The name of the
        file must follow the c immediately without any spaces.  See the
        section called "Config File" for more details about the config
        file.

  -d -> Disables automatic code determination.  When this flag is used,
        DiStella is "dumb" and thinks everything is code unless told
        specifically otherwise in the config file.

  -i -> DiStella will read the address indicated in the last 6th-last
        and 5th-last bytes of the ROM file - the interrupt vector -
        and trace through it to help determine data areas.  This is
        only a valid option if the `-7' flag is set, since this vector
        is only used in Atari 7800 mode.  Usually, access to this vector
        is triggered by a Display list interrupt set in the 7800 DLL.
        Note that not all programs use the interrupt vector.  The best
        thing to do is try disassembling the image without this flag and
        see if the 6th-last and 5th-last bytes point to an area that is
        not disassembled.  If that is the case, try the flag and see if
        the interrupt routine contains valid code.  For 7800 games, most
        games use the interrupt vector.

  -k -> Enables support of the POKEY equates.  There are only 2 actual
        released games that use these equates, which are Ballblazer
        and Commando.  For the rest of the games, this flag is probably
        not very beneficial, as it could translate any bankswitching
        access as POKEY equates (i.e. stores/loads in the 0x4000-0x400F
        range would be identified as POKEY equates, when in reality, they
        may contain bankswitching logic).  Also, note that if using an a78
        file, this is overriden by header byte 54 (zero-based)'s least
        significant bit value.

 -o# -> ORG mnemonic variation.

        o1 -> ORG $XXXX
        o2 -> *=$XXXX
        o3 -> .OR $XXXX

  -r -> Relocate calls out of normal address range.  Only the lowest 13
        bits in an address are significant in the 2600, so $1000 is
        equivalent to $f000.  Unfortunately, some ROM images use these
        addresses interchangeably.  If this flag is NOT used, a section of
        code may look like this:

        LF000 lda $D004 ; this actually refers to LF004
              rts
        LF004 .byte $3c

        If this flag IS used, the same code would look like this:

        LF000 lda LF004 ; ahh! This is a little clearer :)
              rts
        LF004 .byte $3c

        It is important to note that if the -r flag is used, the code will
        recompile fine, but the ROM image will be altered.  If you want your
        source to recompile into an exact copy of the original ROM image, do
        not use this flag!

  -s -> Includes the cycle count for each instruction.  It only includes
        the basic cycle count, and does not adjust for page boundries (YET).

Config File

    The config file is a very simple text file that defines various
parameters for disassembly.  Each line in the config file defines either a
range of addresses or the ORG.  The addresses should be 4 digit hex numbers
and there should be only 1 space between the command and each address.

The valid config commands are as follows:

    ORG XXXX

        Defines where the ROM image should be disassembled to.  Distella
    automatically determines the origin of the ROM image.  It takes into
    consideration the start address (the address specified in lo/hi byte
    format starting from the 4th byte from the end of the ROM image), and
    the length of the image.  The ORG command will override DiStella's
    automatic determination.  Be careful, if you are wrong, you won't get
    much in the way of code on your output!

    CODE XXXX XXXX

        Defines an address range as being code.  Distella is not perfect,
    and can mistake code for data.  The most common way that this happens
    is when Absolute Indirect addressing is used.  See the Limitations
    section for more information.

        The CODE command overrides DiStella's automatic DATA determination,
    but is overridden by all other config commands, so if there are any
    conflicts with DATA or ORG commands, the range in question will not be
    handled as code.

    GFX XXXX XXXX

        Defines an address range as being graphics.  This causes each byte
    to be displayed visually in a comment, along with the address of each
    byte to the right of the graphic display.

        Here is an example from Pacman:


         .byte $38 ; |  XXX   | $FDB5
         .byte $7C ; | XXXXX  | $FDB6
         .byte $FE ; |XXXXXXX | $FDB7
         .byte $E0 ; |XXX     | $FDB8
         .byte $FE ; |XXXXXXX | $FDB9
         .byte $6C ; | XX XX  | $FDBA
         .byte $38 ; |  XXX   | $FDBB
         .byte $7E ; | XXXXXX | $FDBC
         .byte $E0 ; |XXX     | $FDBD
         .byte $C0 ; |XX      | $FDBE
         .byte $E0 ; |XXX     | $FDBF
         .byte $6C ; | XX XX  | $FDC0
         .byte $38 ; |  XXX   | $FDC1

        Note that the graphics are upside down, this is common in games
    because of the way the code is written.

        The GFX command overrides the DATA command.

    DATA XXXX XXXX

        Defines an address range as being data.  Up to 16 bytes will be
    put on each line.  If an address is reached that is referenced somewhere
    else in the code, a new line will be created with its own .byte mnemonic.
    Here is an example from Pacman:

    LFF06: .byte $20,$40,$80,$60,$01,$05
    LFF0C: .byte $00
    LFF0D: .byte $00,$00,$01,$00,$00,$01,$06,$05,$04,$03,$02,$01
    LFF19: .byte $00,$02

Limitations:

    DiStella does a good job at determining the difference between code and
data.  There are a couple of instances that may cause DiStella to confuse
code for data and visa-versa.

    Absolute-Indirect Addressing:

        DiStella traces the code in the ROM image starting at the reset
    vector.  Each time it sees a relative branch, it puts the branch address
    in a queue only if that address hasn't been traced already.  It continues
    on until it reaches an RTS, RTI, or JMP.  It then gets an address from
    the queue and traces it repeating the process until no more addresses
    are in the queue.  Unfortunately, an absolute-indirect JMP - jmp ($ZP) -
    doesn't provide enough information for DiStella.  It is possible that
    an entire section of the ROM image will be determined to be DATA when
    it is really CODE.  The best thing to do is to look at the code that
    loads $ZP and $ZP+1 with the address to jump to (usually there is a list
    of address like .byte $00,$f0,$20,$f0,$30,$f0) and use the CODE command
    in a config file to force that area to be disassembled.

    Relative "Unconditional" Branches:

        Well, as you may or may not know, the 6502 processor does not have
    a relative unconditional branch.  Programmers can cheat and use a
    relative branch as an absolute branch when they know the status of one
    of the status register bits.  For example, if this code is executed:

                        LDA  #$01
                        BNE  LF034
                 LF030  .byte $10,$20,$30,$40
                 LF034  RTS

        LF030 will never be reached because 1 is never equal to zero!
    DiStella isn't that smart!  If you get a large section of code that is
    unreadable, look for a relative branch directly before the unreadable
    section.  Chances are that the data starts directly after that relative
    branch.  You would need to use the DATA command in a config file to fix
    this problem.

    RTS Ending An Interrupt Routing:

        Usually a BRK initiates an interrupt and the code pointed to by the
    interrupt vector is executed.  The proper way of exiting an interrupt is
    to execute an RTI.  A problem occurs when an RTS is executed instead.
    This is because an RTS pulls the return address off of the stack and then
    adds 1 to it, where an RTI does not.  What does this mean to you?  Well,
    look at the following code:

                    LF000 LDA #$01
                          BRK
                          .byte $03 ;.SLO
                    LF004 TAX

        This is an example of what the code might look like if an RTS is
    used in the interrupt routine instead of RTI.  If an RTI was used, the
    code would look like this:

                    LF000 LDA #$01
                          BRK
                    LF003 TAX
                    
        The extra byte in the first code segment could cause DiStella to
    get off sync, which has numerous side effects.  Here it didn't, but keep
    in mind that it is possible.

That's it!  Have fun, and report all bugs and errors to:

   Bob Colbert    - rcolbert@novia.net
                    http://www.atari2600collector.com

   Dan Boris      - dboris@comcast.net
                    http://www.atarihq.com/danb

For version 3.0 and Atari 7800-related errors, contact:

   John K. Harvey - jkharvey@voyager.net
                    http://www.cs.wisc.edu/~harvey/7800/



