.globl _start
.text
_start:
lui sp, 0x80;
j main;  # jump to main
