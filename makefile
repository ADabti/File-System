all: load_fs1 load_fs2 load_fs3 create_fs1 create_fs2 create_fs3

load_fs1: load_fs1.o inode.o allocation.o
	gcc -g -Wall -Wextra $^ -o $@

load_fs2: load_fs2.o inode.o allocation.o
	gcc -g -Wall -Wextra $^ -o $@

load_fs3: load_fs3.o inode.o allocation.o
	gcc -g -Wall -Wextra $^ -o $@

create_fs1: create_fs1.o inode.o allocation.o
	gcc -g -Wall -Wextra $^ -o $@

create_fs2: create_fs2.o inode.o allocation.o
	gcc -g -Wall -Wextra $^ -o $@

create_fs3: create_fs3.o inode.o allocation.o
	gcc -g -Wall -Wextra $^ -o $@

load_fs1.o: load_example1/load_fs.c
	gcc -I. -g -Wall -Wextra -c $^ -o $@

load_fs2.o: load_example2/load_fs.c
	gcc -I. -g -Wall -Wextra -c $^ -o $@

load_fs3.o: load_example3/load_fs.c
	gcc -I. -g -Wall -Wextra -c $^ -o $@

create_fs1.o: create_example1/create_fs.c
	gcc -I. -g -Wall -Wextra -c $^ -o $@

create_fs2.o: create_example2/create_fs.c
	gcc -I. -g -Wall -Wextra -c $^ -o $@

create_fs3.o: create_example3/create_fs.c
	gcc -I. -g -Wall -Wextra -c $^ -o $@

%.o: %.c
	gcc -g -c $^

clean:
	rm -rf *.o
	rm -rf *.dSYM
	rm -f check_disk
	rm -f create_fs load_fs
