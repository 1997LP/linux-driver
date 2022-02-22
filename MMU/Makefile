objs := start.o main.o
mmu.bin:$(objs)
	arm-linux-gnueabihf-ld -Tmmu.lds -o mmu.elf $^
	arm-linux-gnueabihf-objcopy -O binary -S mmu.elf $@
	arm-linux-gnueabihf-objdump -D mmu.elf > mmu.dis
%.o:%.s
	arm-linux-gnueabihf-gcc -Wall -nostdlib -c -O2 -o $@ $<
%.o:%.S
	arm-linux-gnueabihf-gcc -Wall -nostdlib -c -O2 -o $@ $<
%.o:%.c
	arm-linux-gnueabihf-gcc -Wall -nostdlib -c -O2 -o $@ $<
clean:
	rm -rf *.o mmu.bin mmu.elf mmu.dis



    
