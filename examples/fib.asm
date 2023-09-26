  ;; -*- mode: asm -*-

  ;; Initial setup
  push 1
  push 1

  ;; Main loop
  label recur
  dup 1
  dup 1
  plus

  push *1
  jmp print-data
  jmp recur

  ;; Routine to print with newline the top of the stack (without
  ;; altering data) then returning to caller
  label print-data
  dup 1
  print
  pop
  push '\n'
  print
  pop
  jmp *

  halt
