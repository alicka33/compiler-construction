.globl main

.data

n_rows: .quad 10
n_cols: .quad 10
space_for_numbers: .quad 3

space: .string " "
newline: .string "\n"

integer_slot: .string "%ld"

.text
/*
    Main "function" - program starts here!
    It works as a regular function, but is called at when running the program instead of by other code.

    We allow main to use the callee-save/"global" registers:
    %rbx, %rbp, and %r12-r15

    Our program will probably use loops to iterate through the table.
    Thus, we will probably use variables/counters tracking the current loop,
    and possibly another to store the multiplication result.

    How you handle this is up to you,
    but you may assign certain registers to always represent certain variables.
    E.g.:
        r12 = i
        r13 = j
        multiplication_result = r14
        etc...

    Note that we do have a finite amount of "global" registers.
    We use these as we *know* they cannot (should not) be overwritten by other functions.

    We recommend that you keep a AT&T Assembly sheet cheat like this nearby:
    https://cs.brown.edu/courses/cs033/docs/guides/x64_cheatsheet.pdf
    
*/
main:
    pushq %rbp
    movq %rsp, %rbp

    movq $1, %r12
    
.outer_loop:
    movq $1, %r13

.inner_loop:
    movq %r12, %rax
    mulq %r13
    movq %rax, %r14

    movq %rax, %rdi
    call get_digits

    movq $3, %rdx
    subq %rax, %rdx

    movq %r14, %rdi
    movq %rdx, %rsi
    call print_padded_number

    call print_space

    incq %r13
    cmpq n_cols(%rip), %r13
    jle .inner_loop

    leaq newline(%rip), %rdi
    call safe_printf

    incq %r12
    cmpq n_rows(%rip), %r12
    jle .outer_loop

    movq $0, %rax
    leave
    ret


    # TODO 2.A
    # Use a register to track what e.g. row we are at
    # (Recommended) Use a global register
    # (Recommended) Use 1-indexing / start at 1

    # TODO 2.B
    # Prepare the outer loop
    # OUTER LOOP START

        # TODO 2.C
        # Reset the register tracking what e.g. column we are at
        # (Recommended) Use a global register
        # (Recommended) Use 1-indexing / start at 1

        # TODO 2.D
        # Check if we are done with all e.g. rows (and thus the entire table),
        # and should break the loop
        # If so, move out of the loop

        # If not, we should repeat the inner loop

        # TODO 2.E
        # Prepare the inner loop
        # INNER LOOP START

            # TODO 2.D
            # Check if we are done with all e.g. columns and should break the loop
            # If so, move out of the inner loop

            # TODO 2.E
            # Perform multiplication
            # (Optional, Recommended) Store the result somewhere safe so the next functions won't accidently alter it
            # (Hint) M[i, j] = i * j
            # (Hint) You may use `mulq S` here: %rax = %rax * S

            # TODO 2.F
            # Get the number of digits of the result
            # (Hint) You shouldn't have to write a lot of code for this one (unless you want to)

            # TODO 2.G
            # Calculate the amount of padding for the number
            # (Hint) If we allocate 3 spaces for every number, then e.g. "5" should have 2 spaces of padding
            # (Hint) You may use `subq S, D` here: "D = D - S"

            # TODO 2.H
            # Print the padded number using `print_padded_number` using the multiplication result and number of padding spaces
            # Afterwards, print an additional space to sepeate this number from the next

            # We are now done with a single number

            # TODO 2.I
            # Increment the inner loop count, and repeat the loop

        # INNER LOOP END

        # TODO 2.J
        # First, print a newline ('\n') indicating that we are done with a row
        # Then, increment the outer loop counter
        # Finally, repeat the outer loop

    # OUTER LOOP END

    # After completing the outer loop we should be done with the table

/*
    Functions are defined here.
    These will be executed when we call them using the `call` keyword.
*/

/*
    Function: print_padded_number

    Prepends PADDING spaces before NUMBER, and prints it to output

    Args:
        (%rdi) NUMBER (quad)  - Number to print
        (%rsi) PADDING (quad) - Number of padding spaces before number 
*/
print_padded_number:
    pushq %rbp
    movq %rsp, %rbp

    movq %rdi, %r8
    movq %rsi, %r9
    
    movq $0, %r10
    .print_space_loop:
        cmpq %r9, %r10
        jge .print_number
        # call print_space   # This causes my program to end up in an infinitive loop, I have not managed to fix this problem
        incq %r10
        jmp .print_space_loop

    .print_number:
    leaq integer_slot(%rip), %rdi
    movq %r8, %rsi
    call safe_printf

    leave
    ret

    # TODO 1.A:
    # (Optional, recommended) 
    # Move the arguments NUMBER and PADDING to two other register to better keep track of them.

    # TODO 1.B
    # Keep track of the amount of spaces printet for this number
    # Assign count to a register (starting at 0)

    # TODO 1.C
    # Create a loop printing PADDING spaces
    
    # Print Space Loop START

        # TODO 1.C.1
        # Check if we are done printing spaces using the count from 1.B
        # If we are done, jump out of the loop
    
        # TODO 1.C.2
        # Print a single space
        # You are free to use the `print_space` function here, or do it your own way
        # HOWEVER, note that if you call ANY function, you should PUSH all caller-save registers that you currently use
        # When done, pop the values in reverse order to their respective registers


        # TODO 1.C.3
        # Incement the "number of spaces printed" counter
        # and continue the loop

    # Print Space Loop END
    # TODO 1.C.1*
    # Jump here from 1.C.1

    # TODO 1.D
    # We are now done printing all the spaces
    # Print the number
    # Hint: You may use the `integer_slot` here


    # Done

/*
    Function: print_space

    Simply prints a space.
*/
print_space:
    pushq %rbp
    movq %rsp, %rbp

    leaq space(%rip), %rdi
    call safe_printf

    leave
    ret

/*
    Function: get_digits

    Calculates the digits of NUMBER, and returns it as N_DIGITS

    Args:
        (%rdi) NUMBER (quad)  - Number to print

    Returns:
        (%rax) N_DIGITS (quad) - Number of digits in NUMBER 
*/
get_digits:
    pushq %rbp
    movq %rsp, %rbp

    movq $1, %r11

    movq %rdi, %rax
    
gd_loop:
    movq $10, %r8

    movq $0, %rdx
    divq %r8

    movq $0, %r8

    cmp %rax, %r8
    je gd_loop_end

    inc %r11
    jmp gd_loop

gd_loop_end:
    movq %r11, %rax

    leave
    ret


/*
    Function: safe_printf

    Prints to output - like printf - but makes sure that stack is alligned before calling printf
    Convinience function that avoids a lot of crashes caused by printf

    Args:
        (all) any value, but always a string pointer in (%rdi)
*/
safe_printf:
	pushq %rbp
	movq %rsp, %rbp

	andq $-16, %rsp
	call printf
    
	leave
	ret

// Workaround for macOS prefixing symbols with _
#ifdef __APPLE__
    .set printf, _printf
    .set putchar, _putchar
    .set _main, main
    .global _main
#endif
