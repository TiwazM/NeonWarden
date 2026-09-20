begin_cinematic:
 lda #3
 sta stage
 jsr load_stage
 lda #0
 sta camera
 sta camera+1
 sta cine_time
 lda #4
 sta state
 jmp sfx
