OBJS = utils.o ctr_crypto.o yaml.o settings.o main.o srl.o ncsd.o ncch.o cia.o ticket.o tmd.o meta.o
POLAR_OBJS = polarssl/aes.o polarssl/bignum.o polarssl/rsa.o polarssl/sha2.o
LIBS = -static-libgcc -static-libstdc++
CXXFLAGS = -I.
CFLAGS = --std=c99 -Wall -I.
OUTPUT = make_cia
CC = gcc

main: $(OBJS) $(POLAR_OBJS)
	g++ -o $(OUTPUT) $(LIBS) $(OBJS) $(POLAR_OBJS)


clean:
	rm -rf $(OUTPUT) $(OBJS) $(POLAR_OBJS)