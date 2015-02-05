CFLAGS = -g
objects = my_ipmsg.o

all: my_ipmsg

my_ipmsg: $(objects)
	gcc $(CFLAGS) -o my_ipmsg $(objects)

my_ipmsg.o: my_ipmsg.c ipmsg.h my_ipmsg.h
	gcc $(CFLAGS) -c $^

clean:
	rm my_ipmsg.o my_ipmsg *.gch
