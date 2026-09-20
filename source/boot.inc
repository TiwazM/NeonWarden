; NEON WARDEN - BASIC 65 machine-code loader, adapted from supplied ION DRIFT.
; Assemble with 64tass 1.60+.
.cpu "45gs02"
.enc "ascii"
.cdef " ~",32
* = $2001
 .word basic_end
 .word 10
 .byte $fe,$02,$30,':',$9e
 .text "8256"
 .byte 0
basic_end: .word 0
* = $2040

