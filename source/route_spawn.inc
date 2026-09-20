; Keep eight active enemy slots; move the last guards into the new sections.
route_spawn:
 lda stage
 cmp #2
 bcc route_spawn_done
 cmp #5
 bcs route_spawn_done
 cpx #7
 bne route_guard
 lda #<928
 sta ex,x
 sta home,x
 lda #>928
 sta exhi,x
 sta homehi,x
 lda #132
 sta ey,x
 rts
route_guard:
 cpx #6
 bne route_spawn_done
 ; Archive summit guard / rooftop upper guard / Coil lower landing guard.
 ldy stage
 lda route_guard_lo-2,y
 sta ex,x
 sta home,x
 lda #3
 sta exhi,x
 sta homehi,x
 lda route_guard_y-2,y
 sta ey,x
route_spawn_done:
 rts
route_guard_lo: .byte <800,<840,<904
route_guard_y: .byte 40,40,104
