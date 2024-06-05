symnmf: symnmf.o symnmf.h
	@echo "Building symnmf"
	@gcc -o symnmf symnmf.o -lm

symnmf.o: symnmf.c
	@echo "Compiling symnmf.c"
	@gcc -ansi -Wall -Wextra -Werror -pedantic-errors -c symnmf.c 

clean:
	@echo "Cleaning up"
	@rm -f *.o symnmf
