## ======================================================================
## partial Makefile provided to students
##

GTK_INCLUDE := `pkg-config --cflags gtk+-3.0`
GTK_LIBS := `pkg-config --libs gtk+-3.0`


.PHONY: clean new style feedback submit1 submit2 submit

CFLAGS += -std=c11 -Wall -pedantic -g 

# a bit more checks if you'd like to (uncomment
# CFLAGS += -Wextra -Wfloat-equal -Wshadow                         \
# -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings \
# -Wconversion -Wunreachable-code

# uncomment if you want to add DEBUG flag
# CPPFLAGS += -DDEBUG

# ----------------------------------------------------------------------
# feel free to update/modifiy this part as you wisha

# all those libs are required on Debian, feel free to adapt it to your box
LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

all::gbsimulator

TARGETS := 
CHECK_TARGETS := 
OBJS = 
OBJS_NO_STATIC_TESTS =
OBJS_STATIC_TESTS = 
OBJS = $(OBJS_STATIC_TESTS) $(OBJS_NO_STATIC_TESTS)

# ----------------------------------------------------------------------
# This part is to make your life easier. See handouts how to make use of it.


clean::
	-@/bin/rm -f *.o *~ $(CHECK_TARGETS)

new: clean all

static-check:
	scan-build -analyze-headers --status-bugs -maxloop 64 make CC=clang new

style:
	astyle -n -o -A8 -xt0 *.[ch]

# all those libs are required on Debian, adapt to your box
$(CHECK_TARGETS): LDLIBS += -lcheck -lm -lrt -pthread -lsubunit

check:: $(CHECK_TARGETS)
	$(foreach target,$(CHECK_TARGETS),./$(target) &&) true

# target to run tests
check:: all
	#@if ls tests/*.*.sh 1> /dev/null 2>&1; then \
	 # for file in tests/*.*.sh; do [ -x $$file ] || echo "Launching $$file"; ./$$file || exit 1; done; \
	#fi
all:: unit-test-alu unit-test-bit unit-test-bit-vector unit-test-bus unit-test-cartridge unit-test-component unit-test-cpu unit-test-cpu-dispatch-week08 unit-test-cpu-dispatch-week09 unit-test-memory unit-test-timer test-cpu-week08 test-gameboy gbsimulator test-cpu-week09

unit-test-bit-vector: unit-test-bit-vector.o error.o image.o bit.o bit_vector.o 
	gcc unit-test-bit-vector.o error.o image.o bit.o bit_vector.o -lcheck -lm -lrt -pthread -lsubunit -o unit-test-bit-vector


gbsimulator: CFLAGS += $(GTK_INCLUDE)
gbsimulator: LDFLAGS += -L.
gbsimulator: LDLIBS += $(GTK_LIBS) -lsid -lcs212gbfinalext-debug
gbsimulator: gbsimulator.c libsid.so libcs212gbfinalext-debug.so lcdc.h joypad.h gameboy.o bus.o memory.o\
 component.o cpu.o alu.o bit.o timer.o cartridge.o util.o error.o cpu-registers.o\
 cpu-alu.o opcode.o bootrom.o bit_vector.o image.o
	$(CC) -D_DEFAULT_SOURCE -g3 $(CFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

test-image : test-image.o error.o image.o bit.o bit_vector.o libsid.so
	gcc test-image.o error.o image.o bit.o bit_vector.o libsid.so -lsid -L. $(GTK_LIBS) -o test-image 


unit-test-component:unit-test-component.o error.o component.o memory.o bus.o bit.o
	gcc unit-test-component.o error.o bus.o component.o memory.o bit.o -lcheck -lm -lrt -pthread -lsubunit -o unit-test-component

unit-test-bus:unit-test-bus.o error.o bus.o component.o bit.o memory.o
	gcc unit-test-bus.o error.o bus.o component.o bit.o memory.o -lcheck -lm -lrt -pthread -lsubunit -o unit-test-bus

unit-test-memory:unit-test-memory.o error.o component.o memory.o bus.o bit.o
	gcc unit-test-memory.o error.o bus.o component.o memory.o bit.o -lcheck -lm -lrt -pthread -lsubunit -o unit-test-memory

unit-test-cpu:unit-test-cpu.o error.o component.o memory.o bus.o bit.o alu.o cpu-registers.o cpu-storage.o cpu-alu.o cpu.o opcode.o
	gcc -L. unit-test-cpu.o error.o component.o memory.o bus.o bit.o alu.o cpu-registers.o cpu-storage.o cpu.o cpu-alu.o opcode.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cpu
unit-test-bit:unit-test-bit.o error.o bit.o
	gcc unit-test-bit.o error.o bit.o -lcheck -lm -lrt -pthread -lsubunit -o unit-test-bit
unit-test-alu:unit-test-alu.o error.o alu.o bit.o 
	gcc unit-test-alu.o error.o alu.o bit.o -lcheck -lm -lrt -pthread -lsubunit -o unit-test-alu

unit-test-cpu-dispatch-week08:unit-test-cpu-dispatch-week08.o error.o component.o memory.o bus.o bit.o alu.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o
	gcc -L. unit-test-cpu-dispatch-week08.o error.o component.o memory.o bus.o bit.o alu.o cpu-registers.o cpu-storage.o  cpu-alu.o opcode.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cpu-dispatch-week08

unit-test-cpu-dispatch-week09:unit-test-cpu-dispatch-week09.o error.o component.o memory.o bus.o bit.o alu.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o
	gcc -L. unit-test-cpu-dispatch-week09.o error.o component.o memory.o bus.o bit.o alu.o cpu-registers.o cpu-storage.o  cpu-alu.o opcode.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cpu-dispatch-week09

test-cpu-week08: test-cpu-week08.c opcode.o bit.o cpu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o util.o cpu-alu.o error.o
	gcc -L. test-cpu-week08.c opcode.o bit.o cpu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o cpu-alu.o util.o error.o -lcs212gbcpuext -o test-cpu-week08

test-cpu-week09: test-cpu-week09.c opcode.o bit.o cpu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o util.o cpu-alu.o error.o
	gcc -L. test-cpu-week09.c opcode.o bit.o cpu.o alu.o bus.o memory.o component.o cpu-storage.o cpu-registers.o cpu-alu.o util.o error.o -lcs212gbcpuext -o test-cpu-week09


unit-test-cartridge : unit-test-cartridge.o error.o cartridge.o cpu.o bus.o component.o memory.o bit.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o alu.o util.o
	gcc -L. unit-test-cartridge.o error.o cartridge.o cpu.o bus.o component.o memory.o bit.o cpu-registers.o cpu-storage.o cpu-alu.o opcode.o alu.o util.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-cartridge


unit-test-timer : unit-test-timer.o error.o cpu.o bus.o component.o memory.o bit.o cpu-registers.o cpu-alu.o opcode.o timer.o alu.o util.o
	gcc -L. unit-test-timer.o error.o cpu.o bus.o component.o memory.o bit.o cpu-registers.o cpu-alu.o opcode.o timer.o alu.o util.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-timer


test-gameboy: CPPFLAGS += -DBLARGG
test-gameboy: CFLAGS += $(GTK_INCLUDE)
test-gameboy: LDFLAGS += -L.
test-gameboy: LDLIBS += $(GTK_LIBS) -lsid -lcs212gbfinalext-debug
test-gameboy: test-gameboy.c gameboy.o bus.o memory.o component.o cpu.o \
 alu.o bit.o timer.o cartridge.o error.o bootrom.o util.o cpu-registers.o cpu-alu.o opcode.o image.o bit_vector.o
	$(CC) -D_DEFAULT_SOURCE -g3 $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)

unit-test-alu_ext :unit-test-alu_ext.o cpu-storage.o cpu-registers.o cpu-alu.o alu.o bus.o bit.o error.o
	gcc -L . unit-test-alu_ext.o cpu-storage.o cpu-registers.o cpu-alu.o alu.o bus.o bit.o error.o -lcs212gbcpuext -lcheck -lm -lrt -pthread -lsubunit -o unit-test-alu_ext

unit-test-cpu-dispatch : unit-test-cpu-dispatch.o cpu-storage.o cpu-registers.o cpu-alu.o opcode.o alu.o component.o memory.o bus.o bit.o error.o
	gcc -L . unit-test-cpu-dispatch.o cpu-storage.o cpu-registers.o cpu-alu.o opcode.o alu.o component.o memory.o bus.o bit.o error.o -lcs212gbcpuext  -lcheck -lm -lrt -pthread -lsubunit  -o unit-test-cpu-dispatch



IMAGE=chappeli/feedback:latest
feedback:
	@docker pull $(IMAGE)
	@docker run -it --rm -v ${PWD}:/home/tester/done $(IMAGE)

SUBMIT_SCRIPT=../provided/submit.sh
submit1: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 1

submit2: $(SUBMIT_SCRIPT)
	@$(SUBMIT_SCRIPT) 2

submit:
	@printf 'what "make submit"??\nIt'\''s either "make submit1" or "make submit2"...\n'

libsid.so: sidlib.c
	$(CC) -fPIC -shared $(CPPFLAGS) $(CFLAGS) $(GTK_INCLUDE) $^ -o $@

libsid_demo.o: CFLAGS += $(GTK_INCLUDE)

libsid_demo: LDFLAGS += -L.
libsid_demo: LDLIBS += $(GTK_LIBS) -lsid
libsid_demo: libsid_demo.o libsid.so


##// copié depuis gcc -MM et enleve les mauvais tests
alu.o: alu.c alu.h bit.h error.h
bit.o: bit.c bit.h
bit_vector.o: bit_vector.c bit_vector.h bit.h error.h
bootrom.o: bootrom.c bootrom.h bus.h memory.h component.h gameboy.h cpu.h \
 alu.h bit.h util.h cartridge.h timer.h error.h cpu-storage.h opcode.h
bus.o: bus.c bus.h memory.h component.h error.h bit.h
cartridge.o: cartridge.c cartridge.h component.h memory.h bus.h error.h
component.o: component.c component.h memory.h error.h util.h

cpu-alu.o: cpu-alu.c error.h bit.h alu.h cpu-alu.h opcode.h cpu.h bus.h \
 memory.h component.h cpu-storage.h cpu-registers.h
cpu.o: cpu.c error.h opcode.h bit.h cpu.h alu.h bus.h memory.h \
 component.h cpu-alu.h cpu-registers.h cpu-storage.h util.h


cpu-registers.o: cpu-registers.c cpu-registers.h cpu.h alu.h bit.h bus.h \
 memory.h component.h error.h
cpu-storage.o: cpu-storage.c error.h cpu-storage.h memory.h opcode.h \
 bit.h cpu.h alu.h bus.h component.h cpu-registers.h gameboy.h util.h \
 cartridge.h timer.h
error.o: error.c
gameboy.o: gameboy.c gameboy.h bus.h memory.h component.h cpu.h alu.h \
 bit.h util.h cartridge.h timer.h joypad.h lcdc.h image.h bit_vector.h \
 error.h bootrom.h
image.o: image.c error.h image.h bit_vector.h bit.h
libsid_demo.o: libsid_demo.c sidlib.h
memory.o: memory.c memory.h error.h
opcode.o: opcode.c opcode.h bit.h
sidlib.o: sidlib.c sidlib.h
test-cpu-week08.o: test-cpu-week08.c opcode.h bit.h cpu.h alu.h bus.h \
 memory.h component.h cpu-storage.h util.h error.h
test-cpu-week09.o: test-cpu-week09.c opcode.h bit.h cpu.h alu.h bus.h \
 memory.h component.h cpu-storage.h util.h error.h
test-gameboy.o: test-gameboy.c gameboy.h bus.h memory.h component.h cpu.h \
 alu.h bit.h util.h cartridge.h timer.h error.h lcdc.h joypad.h
timer.o: timer.c timer.h component.h memory.h bit.h cpu.h alu.h bus.h \
 error.h cpu-storage.c cpu-storage.h opcode.h cpu-registers.h gameboy.h \
 util.h cartridge.h
unit-test-alu.o: unit-test-alu.c tests.h error.h alu.h bit.h
unit-test-alu_ext.o: unit-test-alu_ext.c tests.h error.h alu.h bit.h \
 alu_ext.h
unit-test-bit.o: unit-test-bit.c tests.h error.h bit.h
unit-test-bit-vector.o: unit-test-bit-vector.c tests.h error.h \
 bit_vector.h bit.h image.h
unit-test-bus.o: unit-test-bus.c tests.h error.h bus.h memory.h \
 component.h util.h
unit-test-cartridge.o: unit-test-cartridge.c tests.h error.h cartridge.h \
 component.h memory.h bus.h cpu.h alu.h bit.h
unit-test-component.o: unit-test-component.c tests.h error.h bus.h \
 memory.h component.h
unit-test-cpu.o: unit-test-cpu.c tests.h error.h alu.h bit.h opcode.h \
 util.h cpu.h bus.h memory.h component.h cpu-registers.h cpu-storage.h \
 cpu-alu.h
unit-test-cpu-dispatch.o: unit-test-cpu-dispatch.c tests.h error.h alu.h \
 bit.h cpu.h bus.h memory.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h
unit-test-cpu-dispatch-week08.o: unit-test-cpu-dispatch-week08.c tests.h \
 error.h alu.h bit.h cpu.h bus.h memory.h component.h opcode.h gameboy.h \
 util.h cartridge.h timer.h cpu-registers.h cpu-storage.h cpu-alu.h \
 unit-test-cpu-dispatch.h cpu.c
unit-test-cpu-dispatch-week09.o: unit-test-cpu-dispatch-week09.c tests.h \
 error.h alu.h bit.h cpu.h bus.h memory.h component.h opcode.h util.h \
 unit-test-cpu-dispatch.h cpu.c cpu-alu.h cpu-registers.h cpu-storage.h
unit-test-memory.o: unit-test-memory.c tests.h error.h bus.h memory.h \
 component.h
unit-test-timer.o: unit-test-timer.c util.h tests.h error.h timer.h \
 component.h memory.h bit.h cpu.h alu.h bus.h
util.o: util.c
test-image.o : CFLAGS += $(GTK_INCLUDE)
test-image.o: test-image.c error.h util.h image.h bit_vector.h bit.h \
 sidlib.h
unit-test-bit-vector.o: unit-test-bit-vector.c tests.h error.h \
 bit_vector.h bit.h image.h

gbsimulator.o: gbsimulator.c sidlib.h lcdc.h cpu.h alu.h bit.h bus.h \
 memory.h component.h image.h bit_vector.h gameboy.h util.h cartridge.h \
 timer.h error.h








