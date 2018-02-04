all:
	ssh tom@192.168.56.2 'cd /home/tom/fs/hash_fs; ./make.sh'

clean:
	ssh tom@192.168.56.2 'cd /home/tom/fs/hash_fs; ./make.sh clean'

test:
	ssh tom@192.168.56.2 'cd /home/tom/fs/hash_fs; ./test.sh'

