#ifndef SEQDIA_TYPES_H
#define SEQDIA_TYPES_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  ARROW_SOLID,
  ARROW_SOLID_LEFT,
  ARROW_OPEN,
  ARROW_OPEN_LEFT,
  ARROW_DASHED,
  ARROW_DASHED_LEFT,
  ARROW_X,
  ARROW_X_LEFT
} ArrowType;

typedef enum { NOTE_LEFT, NOTE_RIGHT, NOTE_OVER } NotePosition;

typedef enum { EVENT_MESSAGE, EVENT_NOTE } EventType;

typedef struct {
  char *name;
  char *alias;
  size_t display_width;
  size_t max_line_width;
  size_t line_count;
} Participant;

typedef struct {
  const char *key;
  size_t index;
} ParticipantIndexEntry;

typedef struct {
  size_t from_idx;
  size_t to_idx;
  ArrowType arrow;
  char *text;
  bool is_self;
  size_t inline_left_note_idx;
  size_t inline_right_note_idx;
} Message;

typedef struct {
  NotePosition position;
  size_t from_idx;
  size_t to_idx;
  char *text;
  size_t max_line_width;
  size_t line_count;
} Note;

typedef struct {
  EventType type;
  size_t index;
} DiagramEvent;

typedef struct {
  Participant *participants;
  size_t participant_count;
  size_t participant_capacity;
  ParticipantIndexEntry *participant_index;
  size_t participant_index_count;
  size_t participant_index_capacity;
  Message *messages;
  size_t message_count;
  size_t message_capacity;
  Note *notes;
  size_t note_count;
  size_t note_capacity;
  DiagramEvent *events;
  size_t event_count;
  size_t event_capacity;
} Diagram;

char *safe_strdup(const char *s);

Participant *participant_new(const char *name, const char *alias);
void participant_free(Participant *p);

Message *message_new(size_t from, size_t to, ArrowType arrow, const char *text);
void message_free(Message *m);

Note *note_new(NotePosition position, size_t from, size_t to, const char *text);
void note_free(Note *note);

Diagram *diagram_new(void);
void diagram_free(Diagram *d);

size_t diagram_add_participant(Diagram *d, const char *name, const char *alias);
size_t diagram_find_participant(Diagram *d, const char *name);
bool diagram_add_message(Diagram *d, size_t from, size_t to, ArrowType arrow,
                         const char *text);
bool diagram_add_note(Diagram *d, NotePosition position, size_t from, size_t to,
                      const char *text);

size_t utf8_display_width(const char *s);
size_t utf8_display_width_n(const char *s, size_t n);

#endif
