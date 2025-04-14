# Compilazione di sm-redis
make: sm-redis.c
	gcc -Wall -Wextra -Og -g sm-redis.c -o sm-redis

# Compilazione di list
list: linked_list.c
	gcc -Wall -Wextra -Og -g linked_list.c -o linked_list

# Compilazione del test per la lista collegata
test_list: linked_list_test.c linked_list.c int_list.c linked_list.h int_list.h
	gcc -Wall -Wextra -Og -g linked_list_test.c linked_list.c int_list.c -o test_list

# Esecuzione del test (opzionale)
run_test: test_list
	./test_list

# Pulizia dei file compilati
clean:
	rm -f sm-redis list test_list *.o
