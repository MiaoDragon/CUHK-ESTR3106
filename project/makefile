CC=gcc
OS=_THISIS_LINUX
VER=_SIMPLIPC
SIMPL_DIR=${SIMPL_HOME}
INCL_DIR=${SIMPL_DIR}/include
CDFLAGS=-c -Wall -I ${INCL_DIR} -D${OS} -D${VER}
LDFLAGS=-L ${SIMPL_DIR}/lib -lsimpl 
LIBS=-lcurses
SOURCES = display_admin game_admin cycle timer courier painter input_admin keyboard

all: ${SOURCES}

%.o: %.c
	${CC} ${CDFLAGS} $? -o $@

timer: timer.o
	${CC} $? -o $@ ${LDFLAGS}

cycle: cycle.o
	${CC} $? -o $@ ${LDFLAGS}

painter: painter.o
	${CC} $? -o $@ ${LIBS} ${LDFLAGS}

courier: courier.o
	${CC} $? -o $@ ${LDFLAGS} 

display_admin: display_admin.o
	${CC} $? -o $@ ${LDFLAGS}

game_admin: game_admin.o
	${CC} $? -o $@ ${LIBS} ${LDFLAGS}

input_admin: input_admin.o
	${CC} $? -o $@ ${LDFLAGS}

keyboard: keyboard.o
	${CC} $? -o $@ ${LIBS} ${LDFLAGS}

clean:
	@rm -f *.o *~ ${SOURCES}

