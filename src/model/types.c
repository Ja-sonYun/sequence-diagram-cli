#include "types.h"
#include "text_utils.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

char *safe_strdup(const char *s) {
  if (!s)
    return NULL;
  size_t len = strlen(s);
  char *dup = malloc(len + 1);
  if (dup) {
    memcpy(dup, s, len + 1);
  }
  return dup;
}

static size_t hash_string(const char *s) {
  size_t hash = 5381;
  unsigned char c;
  while ((c = (unsigned char)*s++)) {
    hash = ((hash << 5) + hash) + c;
  }
  return hash;
}

static size_t next_index_capacity(size_t capacity) {
  return capacity ? capacity * 2 : 16;
}

static bool participant_index_resize(Diagram *d, size_t new_capacity) {
  ParticipantIndexEntry *entries =
      calloc(new_capacity, sizeof(ParticipantIndexEntry));
  if (!entries)
    return false;

  size_t mask = new_capacity - 1;
  for (size_t i = 0; i < d->participant_index_capacity; i++) {
    ParticipantIndexEntry *entry = &d->participant_index[i];
    if (!entry->key)
      continue;
    size_t hash = hash_string(entry->key);
    for (size_t j = 0; j < new_capacity; j++) {
      size_t idx = (hash + j) & mask;
      if (!entries[idx].key) {
        entries[idx] = *entry;
        break;
      }
    }
  }

  free(d->participant_index);
  d->participant_index = entries;
  d->participant_index_capacity = new_capacity;
  return true;
}

static bool participant_index_ensure(Diagram *d, size_t extra) {
  if (d->participant_index_capacity == 0) {
    if (!participant_index_resize(d, next_index_capacity(0)))
      return false;
  }

  size_t required = d->participant_index_count + extra;
  while (required > (d->participant_index_capacity * 7) / 10) {
    if (!participant_index_resize(
            d, next_index_capacity(d->participant_index_capacity)))
      return false;
  }

  return true;
}

static size_t participant_index_find(Diagram *d, const char *key) {
  if (!key || d->participant_index_capacity == 0)
    return (size_t)-1;

  size_t hash = hash_string(key);
  size_t mask = d->participant_index_capacity - 1;
  for (size_t i = 0; i < d->participant_index_capacity; i++) {
    size_t idx = (hash + i) & mask;
    ParticipantIndexEntry *entry = &d->participant_index[idx];
    if (!entry->key)
      return (size_t)-1;
    if (strcmp(entry->key, key) == 0)
      return entry->index;
  }

  return (size_t)-1;
}

static bool participant_index_insert(Diagram *d, const char *key,
                                     size_t index) {
  if (!key || d->participant_index_capacity == 0)
    return true;

  size_t hash = hash_string(key);
  size_t mask = d->participant_index_capacity - 1;
  for (size_t i = 0; i < d->participant_index_capacity; i++) {
    size_t idx = (hash + i) & mask;
    ParticipantIndexEntry *entry = &d->participant_index[idx];
    if (!entry->key) {
      entry->key = key;
      entry->index = index;
      d->participant_index_count++;
      return true;
    }
    if (strcmp(entry->key, key) == 0) {
      entry->index = index;
      return true;
    }
  }

  return false;
}

static size_t utf8_display_width_fallback(const char *s, size_t n) {
  size_t width = 0;
  const unsigned char *p = (const unsigned char *)s;
  const unsigned char *end = p + n;
  while (p < end && *p) {
    if (*p < 0x80) {
      width += 1;
      p += 1;
    } else if ((*p & 0xE0) == 0xC0) {
      width += 2;
      p += 2;
    } else if ((*p & 0xF0) == 0xE0) {
      width += 2;
      p += 3;
    } else if ((*p & 0xF8) == 0xF0) {
      width += 2;
      p += 4;
    } else {
      width += 1;
      p += 1;
    }
  }
  return width;
}

size_t utf8_display_width_n(const char *s, size_t n) {
  if (!s || n == 0)
    return 0;

  mbstate_t state;
  memset(&state, 0, sizeof(state));

  size_t width = 0;
  const char *p = s;
  size_t remaining = n;

  while (remaining > 0 && *p) {
    wchar_t wc;
    errno = 0;
    size_t consumed = mbrtowc(&wc, p, remaining, &state);
    if (consumed == (size_t)-1 || consumed == (size_t)-2) {
      return utf8_display_width_fallback(p, remaining) + width;
    }
    if (consumed == 0) {
      break;
    }
    int wc_width = wcwidth(wc);
    if (wc_width < 0)
      wc_width = 1;
    width += (size_t)wc_width;
    p += consumed;
    remaining -= consumed;
  }

  return width;
}

size_t utf8_display_width(const char *s) {
  if (!s)
    return 0;
  return utf8_display_width_n(s, strlen(s));
}

Participant *participant_new(const char *name, const char *alias) {
  Participant *p = malloc(sizeof(Participant));
  if (!p)
    return NULL;
  p->name = safe_strdup(name);
  if (!p->name) {
    free(p);
    return NULL;
  }
  if (alias) {
    p->alias = safe_strdup(alias);
    if (!p->alias) {
      free(p->name);
      free(p);
      return NULL;
    }
  } else {
    p->alias = NULL;
  }
  p->display_width = utf8_display_width(name);
  p->max_line_width = max_line_width(name);
  p->line_count = count_lines(name);
  return p;
}

void participant_free(Participant *p) {
  if (!p)
    return;
  free(p->name);
  free(p->alias);
}

Message *message_new(size_t from, size_t to, ArrowType arrow,
                     const char *text) {
  Message *m = malloc(sizeof(Message));
  if (!m)
    return NULL;
  m->from_idx = from;
  m->to_idx = to;
  m->arrow = arrow;
  const char *safe_text = text ? text : "";
  m->text = safe_strdup(safe_text);
  if (!m->text) {
    free(m);
    return NULL;
  }
  m->is_self = (from == to);
  m->inline_left_note_idx = (size_t)-1;
  m->inline_right_note_idx = (size_t)-1;
  return m;
}

void message_free(Message *m) {
  if (!m)
    return;
  free(m->text);
}

Note *note_new(NotePosition position, size_t from, size_t to,
               const char *text) {
  Note *note = malloc(sizeof(Note));
  if (!note)
    return NULL;
  note->position = position;
  note->from_idx = from;
  note->to_idx = to;
  const char *safe_text = text ? text : "";
  note->text = safe_strdup(safe_text);
  if (!note->text) {
    free(note);
    return NULL;
  }
  note->max_line_width = max_line_width(note->text);
  note->line_count = count_lines(note->text);
  return note;
}

void note_free(Note *note) {
  if (!note)
    return;
  free(note->text);
}

static bool diagram_events_append(Diagram *d, EventType type, size_t index) {
  if (d->event_count >= d->event_capacity) {
    size_t new_cap = d->event_capacity ? d->event_capacity * 2 : 16;
    DiagramEvent *new_arr = realloc(d->events, sizeof(DiagramEvent) * new_cap);
    if (!new_arr)
      return false;
    d->events = new_arr;
    d->event_capacity = new_cap;
  }
  d->events[d->event_count].type = type;
  d->events[d->event_count].index = index;
  d->event_count++;
  return true;
}

Diagram *diagram_new(void) {
  Diagram *d = malloc(sizeof(Diagram));
  if (!d)
    return NULL;
  d->participant_count = 0;
  d->participant_capacity = 8;
  d->participants = malloc(sizeof(Participant) * d->participant_capacity);
  d->participant_index = NULL;
  d->participant_index_count = 0;
  d->participant_index_capacity = 0;
  d->message_count = 0;
  d->message_capacity = 16;
  d->messages = malloc(sizeof(Message) * d->message_capacity);
  d->notes = NULL;
  d->note_count = 0;
  d->note_capacity = 0;
  d->events = NULL;
  d->event_count = 0;
  d->event_capacity = 0;
  if (!d->participants || !d->messages) {
    free(d->participants);
    free(d->messages);
    free(d->notes);
    free(d->events);
    free(d);
    return NULL;
  }
  return d;
}

void diagram_free(Diagram *d) {
  if (!d)
    return;
  for (size_t i = 0; i < d->participant_count; i++) {
    participant_free(&d->participants[i]);
  }
  free(d->participants);
  free(d->participant_index);
  for (size_t i = 0; i < d->message_count; i++) {
    message_free(&d->messages[i]);
  }
  free(d->messages);
  for (size_t i = 0; i < d->note_count; i++) {
    note_free(&d->notes[i]);
  }
  free(d->notes);
  free(d->events);
  free(d);
}

size_t diagram_find_participant(Diagram *d, const char *name) {
  if (!d || !name)
    return (size_t)-1;

  size_t idx = participant_index_find(d, name);
  if (idx != (size_t)-1 || d->participant_index_capacity != 0)
    return idx;

  for (size_t i = 0; i < d->participant_count; i++) {
    if (d->participants[i].alias &&
        strcmp(d->participants[i].alias, name) == 0) {
      return i;
    }
    if (strcmp(d->participants[i].name, name) == 0) {
      return i;
    }
  }
  return (size_t)-1;
}

size_t diagram_add_participant(Diagram *d, const char *name,
                               const char *alias) {
  size_t existing = diagram_find_participant(d, name);
  if (existing != (size_t)-1) {
    return (size_t)-1;
  }
  if (alias) {
    existing = diagram_find_participant(d, alias);
    if (existing != (size_t)-1) {
      return (size_t)-1;
    }
  }

  if (d->participant_count >= d->participant_capacity) {
    size_t new_cap = d->participant_capacity ? d->participant_capacity * 2 : 8;
    Participant *new_arr =
        realloc(d->participants, sizeof(Participant) * new_cap);
    if (!new_arr)
      return (size_t)-1;
    d->participants = new_arr;
    d->participant_capacity = new_cap;
  }
  Participant *p = participant_new(name, alias);
  if (!p)
    return (size_t)-1;

  size_t new_index = d->participant_count;
  size_t extra_entries = alias ? 2 : 1;
  if (!participant_index_ensure(d, extra_entries)) {
    participant_free(p);
    free(p);
    return (size_t)-1;
  }
  if (!participant_index_insert(d, p->name, new_index) ||
      !participant_index_insert(d, p->alias, new_index)) {
    participant_free(p);
    free(p);
    return (size_t)-1;
  }

  d->participants[new_index] = *p;
  free(p);
  d->participant_count++;
  return new_index;
}

bool diagram_add_message(Diagram *d, size_t from, size_t to, ArrowType arrow,
                         const char *text) {
  if (d->message_count >= d->message_capacity) {
    size_t new_cap = d->message_capacity * 2;
    Message *new_arr = realloc(d->messages, sizeof(Message) * new_cap);
    if (!new_arr)
      return false;
    d->messages = new_arr;
    d->message_capacity = new_cap;
  }
  Message *m = message_new(from, to, arrow, text);
  if (!m)
    return false;
  d->messages[d->message_count] = *m;
  free(m);
  if (!diagram_events_append(d, EVENT_MESSAGE, d->message_count)) {
    message_free(&d->messages[d->message_count]);
    return false;
  }
  d->message_count++;
  return true;
}

bool diagram_add_note(Diagram *d, NotePosition position, size_t from, size_t to,
                      const char *text) {
  if (d->note_count >= d->note_capacity) {
    size_t new_cap = d->note_capacity ? d->note_capacity * 2 : 8;
    Note *new_arr = realloc(d->notes, sizeof(Note) * new_cap);
    if (!new_arr)
      return false;
    d->notes = new_arr;
    d->note_capacity = new_cap;
  }
  Note *note = note_new(position, from, to, text);
  if (!note)
    return false;
  d->notes[d->note_count] = *note;
  free(note);
  if (!diagram_events_append(d, EVENT_NOTE, d->note_count)) {
    note_free(&d->notes[d->note_count]);
    return false;
  }
  d->note_count++;
  return true;
}
