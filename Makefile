OBJ_DIR = obj

CFLAG = -c -Wall -O2 -std=c99
OBJECTS = main.o arrow_connection.o participant.o renderer.o parser.o scanner.o
TARGET = seqdia

ARROW_C_C = ./grammars/arrow_connection.c
PARTICIPANT_C = ./grammars/participant.c
RENDERER_C = ./renderer.c
PARSER_C = ./parser.c
SCANNER_C = ./scanner.c
MAIN_C = ./main.c

.PHONY: clean

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -o $(TARGET)

main.o: $(MAIN_C)
	gcc $(CFLAG) $(MAIN_C)

renderer.o: $(RENDERER_C)
	gcc $(CFLAG) $(RENDERER_C)

parser.o: $(PARSER_C)
	gcc $(CFLAG) $(PARSER_C)

arrow_connection.o: $(ARROW_C_C)
	gcc $(CFLAG) $(ARROW_C_C)

participant.o: $(PARTICIPANT_C)
	gcc $(CFLAG) $(PARTICIPANT_C)

scanner.o: $(SCANNER_C)
	gcc $(CFLAG) $(SCANNER_C)

clean:
	rm -rf seqdia && rm *.o

all: $(TARGET)
