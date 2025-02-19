.globl main
.data

number_of_guesses: .quad 5
guesses: .quad 1, 100, 13, 50, 42
guess_correct: .string "Guessed number %ld is correct!!\n"
guess_too_low: .string "Guessed number %ld is too low.\n"
guess_too_high: .string "Guessed number %ld is too high.\n"

line1: .string "Beth had her troubles as well as the others, and not being an angel but a very human little girl, she often ‘wept a little weep’ as Jo said, because she couldn’t take music lessons and have a fine piano.\n"
line2: .string "She loved music so dearly, tried so hard to learn, and practiced away so patiently at the jingling old instrument, that it did seem as if someone (not to hint Aunt March) ought to help her.\n"
line3: .string "Nobody did, however, and nobody saw Beth wipe the tears off the yellow keys, that wouldn’t keep in tune, when she was all alone.\n"
line4: .string "She sang like a little lark about her work, never was too tired for Marmee and the girls, and day after day said hopefully to herself, “I know I’ll get my music some time, if I’m good.”\n"
line5: .string "There are many Beths in the world, shy and quiet, sitting in corners till needed, and living for others so cheerfully that no one sees the sacrifices till the little cricket on the hearth stops chirping, and the sweet, sunshiny presence vanishes, leaving silence and shadow behind.\n"

n_lines: .quad 5
book: .quad line1, line2, line3, line4, line5

debug: .string "Value is %ld\n"

.text
main:
    pushq %rbp
    movq %rsp, %rbp

    call guess_number

    movq $'\n', %rdi
    call putchar

    call read_book

    movq $0, %rax
    leave
    ret

guess_number:
    pushq %rbp
    movq %rsp, %rbp

    movq $42, %r12                      # Secret number (r12)
    movq $0, %r13                       # i=0 (r13)

    guess_number_loop:
    leaq number_of_guesses(%rip), %rdi  # Load ADDRESS to %rdi
    movq (%rdi), %rdi                   # Dereference %rdi
    cmp %rdi, %r13                      # r13 vs rdi
    jge guess_number_loop_end           # Jump if i >= number_of_guesses
    
    leaq guesses(%rip), %rdi             # Load "guesses" address
    movq (%rdi, %r13, 8), %rdi          # Dereference guess i; guess = guesses[i]
    movq %r12, %rsi                     # Move secret to arg 2 position
    call compare_number                 # Call subfunction

    inc %r13                            # i++
    jmp guess_number_loop
    guess_number_loop_end:

    leave
    ret

/*
    Compare guess in %rdi to secret in %rsi

    Args:
        (%rdi) guess (quad) - One user guess
        (%rsi) secret (quad) - Secret number to guess
*/
compare_number:
    pushq %rbp
    movq %rsp, %rbp

    cmp %rsi, %rdi                      # rdi vs rsi
    movq %rdi, %rsi                     # Preemptively move guess to %rdi
    jg greater                          # Jump to "greater" if guess > secret
    jl lesser                           # Jump to "lesser" if guess < secret
                                        # Otherwise, guess == secret

    # Equal
    leaq guess_correct(%rip), %rdi
    jmp compare_number_end

    # Greater
    greater:
    leaq guess_too_high(%rip), %rdi
    jmp compare_number_end

    # Lesser
    lesser:
    leaq guess_too_low(%rip), %rdi

    compare_number_end:

    call printf                         # Print message and guess

    leave
    ret

read_book:
    pushq %rbp
    movq %rsp, %rbp

    movq $0, %r12
    read_book_loop:
    leaq n_lines(%rip), %rdi            # Reading lines ADDRESS
    movq (%rdi), %rdi                   # Dereferencing lines
    cmp %rdi, %r12                      # r12 vs rdi
    jge read_book_loop_end              # Jump if i >= max

    leaq book(%rip), %rdi               # Reading address of book pointer
    movq (%rdi, %r12, 8), %rdi          # Read nth element; e = lines[i]
    movq $0, %rsi                       # No effect, just for pushing even number of arguments (won't matter here)
    call printf                         # Printing line i; Note printf takes string address so no need for dereferencing

    inc %r12                            # i ++
    jmp read_book_loop
    read_book_loop_end:
    leave
    ret

// Workaround for macOS prefixing symbols with _
#ifdef __APPLE__
    .set printf, _printf
    .set putchar, _putchar
    .set _main, main
    .global _main
#endif
