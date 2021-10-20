OBJ_DIR = obj

# TODO use other func instead of 'strdup'
override CFLAG += -c -Wall -O2 # std=c99
OBJECTS = main.o arrow_connection.o participant.o renderer.o parser.o scanner.o style.o fetch.o
TARGET = seqdia

B_FLAG=-DCLI -DOPTS_SUP
CURLF = -I/usr/local/opt/curl/include -L/usr/local/opt/curl/lib -lcurl

ARROW_C_C = ./grammars/arrow_connection.c
PARTICIPANT_C = ./grammars/participant.c
RENDERER_C = ./renderer.c
PARSER_C = ./parser.c
SCANNER_C = ./scanner.c
MAIN_C = ./main.c
STYLE_C = ./style.c
FETCH_C = ./fetch.c

.PHONY: clean

$(TARGET): $(OBJECTS)
	gcc $(OBJECTS) -o $(TARGET) $(B_FLAG)

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

style.o: $(STYLE_C)
	gcc $(CFLAG) $(STYLE_C)

fetch.o: $(FETCH_C)
	gcc $(CFLAG) $(FETCH_C)

clean:
	rm -f seqdia && rm -f *.o

all: $(TARGET)
