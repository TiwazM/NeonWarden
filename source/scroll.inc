copy0:
 ldz #0
 ldy #0
copyrow0:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 cpz #64
 bne copyrow0
 rts
copy1:
 ldz #0
 ldy #1
copyrow1:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow1
 rts
copy2:
 ldz #0
 ldy #2
copyrow2:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow2
 rts
copy3:
 ldz #0
 ldy #3
copyrow3:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow3
 rts
copy4:
 ldz #0
 ldy #4
copyrow4:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow4
 rts
copy5:
 ldz #0
 ldy #5
copyrow5:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow5
 rts
copy6:
 ldz #0
 ldy #6
copyrow6:
 lda (src),y
 sta [pix],z
 inz
 iny
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow6
 rts
copy7:
 ldz #0
 ldy #7
copyrow7:
 lda (src),y
 sta [pix],z
 inz
 iny
 tya
 sec
 sbc #8
 tay
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 lda (src2),y
 sta [pix],z
 inz
 iny
 tya
 clc
 adc #8
 tay
 cpz #64
 bne copyrow7
 rts
copylo: .byte <copy0,<copy1,<copy2,<copy3,<copy4,<copy5,<copy6,<copy7
copyhi: .byte >copy0,>copy1,>copy2,>copy3,>copy4,>copy5,>copy6,>copy7
