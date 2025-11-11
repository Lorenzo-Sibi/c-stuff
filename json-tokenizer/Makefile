tokenizer: jsont.c
	gcc -Wall -Wextra -Og -g jsont.c -o jsont

parser: jsonp.c jsont.h
	gcc -Wall -Wextra -Og -g jsonp.c -o jsonp

clean:
	rm -f jsont jsonp *.o