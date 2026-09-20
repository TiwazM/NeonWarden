; Decode eight poses once into spare stock RAM, then DMA the selected pose.
; Slots are 768 bytes apart at $14000..$157ff; only 674 bytes are copied.
init_actor_cache:
 lda #0
 sta trace_index
actor_cache_next:
 ldy trace_index
 lda enemy_pose_lo,y
 sta src
 lda enemy_pose_hi,y
 sta src+1
 jsr unpack_actor
 jsr actor_copy
 clc
 lda actor_job+9
 adc #3
 sta actor_job+9
 inc trace_index
 lda trace_index
 cmp #8
 bne actor_cache_next
 lda #0
 sta actor_job+5
 sta actor_job+10
 lda #1
 sta actor_job+7
 lda #<enemy_buffer
 sta actor_job+8
 lda #>enemy_buffer
 sta actor_job+9
 rts
actor_copy:
 lda #0
 sta $d702
 sta $d704
 lda #>actor_job
 sta $d701
 lda #<actor_job
 sta $d705
 rts
actor_job:
 .byte $0a,0,0
 .word 674
 .word enemy_buffer
 .byte 0
 .word $4000
 .byte 1
 .word 0
