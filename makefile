DEBUG=no
CC= gcc
ifeq ($(DEBUG),yes)
	CFLAGS= -W -Wall -ansi -pedantic -g
	LDFLAGS=
else
	CFLAGS= -W -Wall -pedantic
	LDFLAGS=
endif

EXEC= server
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all: $(EXEC)

ifeq ($(DEBUG),yes)
	@echo "Génération en mode debug"
else
	@echo "Génération en mode release"
endif

${EXEC}: $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)


%.o: %.c
	@$(CC) -c $< $(CFLAGS)


######################
# Cibles habituelles #
######################

.PHONY: clean mrproper

clean:
	@rm -f *.o *~

mrproper: clean
	@rm -f $(EXEC)
