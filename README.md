# sequence-diagram-cli

Draw sequence diagrams from the terminal.

## Example

```
$ ./seqdia <<'EOF'
participant Alice
participant Bob
Alice->Bob: Hello
note right of Bob: OK
Alice<-Bob: Bye
note left: end note
EOF
╭───────────────────────────────────╮  ╭───────────────────╮
│               Alice               │  │        Bob        │
╰─────────────────┬─────────────────╯  ╰─────────┬─────────╯
                  ┆                              ┆
                  ┆                              ┆
                  ┆                              ┆
                  ┆            Hello             ┆
                  ├─────────────────────────────▶┆
                  ┆                              ┆ ╒════╕
                  ┆                              ┆ │ OK │
                  ┆                              ┆ └────┘
                  ┆                              ┆
    ╒══════════╕  ┆             Bye              ┆
    │ end note │  ┆◀─────────────────────────────┤
    └──────────┘  ┆                              ┆
                  ┆                              ┆
                  ┆                              ┆
                  ┆                              ┆
                  ┆                              ┆
                  ┆                              ┆

```

See more examples in `tests/fixtures/`.

## Build from Source

```
make
```

## Usage

```
$ seqdia [options] [file]
$ seqdia --style ascii tests/fixtures/simple.txt
$ echo "Alice->Bob: Hello" | seqdia
```

Options:

- `-v, --version` Print version
- `--style <ascii|utf8>` Output style (default: utf8)

## Syntax

### participant

```scala
participant Alice
participant "Billing Service" as Billing
participant "multi
line" as Multi
```

Participants are optional, but defining them keeps columns aligned.

### comment

```scala
// comment
; comment
' comment
```

Comments only work when the line starts with the marker (spaces allowed).

### arrow

```scala
Alice->Bob: solid
Bob<-Alice: solid-left
Alice->>Bob: open
Bob<<-Alice: open-left
Alice-->Bob: dashed
Bob<--Alice: dashed-left
Alice->xBob: x
Bob x<-Alice: x-left
```

### note

```scala
note left of Alice: left note
note right of Bob: right note
note over Alice,Bob: over both
note over Alice
multi
line
end note
```

Multi-line notes must end with `end note`.

Inline notes (attach to the most recent message):

```scala
Alice->Bob: hello
note left: inline left
note right: inline right
```

### TODO

- Installation via brew/nix
- more arrow design
- loop, alt, opt, etc.
- colors
- export to image
