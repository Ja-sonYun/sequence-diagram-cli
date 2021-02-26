# sequence-diagram-cli
Draw seqence diagram from terminal.    
 
<img src="https://github.com/Ja-sonYun/sequence-diagram-cli/blob/main/example.png?raw=true" height="790">
Korean and Japanese(which takes two spaces by character in terminal) will looks like above image only in terminal.     

**You can also customize styles by editing style.h.**

## Installation
 `wget -qO - https://github.com/Ja-sonYun/sequence-diagram-cli/raw/main/install.sh | bash` 
```
Usage

$ ~/sequence-diagram-cli > seqdia 'YOUR_SEQUENCE_DIAGRAM_FILE'
$ ~/sequence-diagram-cli > seqdia 'YOUR_SEQUENCE_DIAGRAM_FILE' prefix='// ' suffix='|' al

v1.1
// with ' al ' option will draw sequence diagram in pure character not utf-8.

//example
$ ~/sequence-diagram-cli > seqdia tests/test.txt
```

## Example ouput
```scala
./tests/test.txt
------------------
; participants
participant User
participant " * TODO
 - clean living room  " as todo
participant Dev

; connections
User->User: default arrow
User-->Dev: styled arrow
todo->todo : "self connecting
with
new line"
User->Dev : yes? sd
User<--Dev : no
todo<--Dev: reverse arrow
S->todo: "EEEE
E
EEEE
E
EEEE"
-------------------

$ ~/sequence-diagram-cli > seqdia tests/test.txt prefix="<-- " suffix=" -->"
===========================================================
<--         ╭────────────────────────╮                  -->
<-- ╭──────╮│  * TODO                │   ╭─────╮╭───╮   -->
<-- │ User ││  - clean living room   │   │ Dev ││ S │   -->
<-- ╰───┬──╯╰────────────┬───────────╯   ╰──┬──╯╰─┬─╯   -->
<--     │                │                  │     │     -->
<--     │                │                  │     │     -->
<--     │ default arrow  │                  │     │     -->
<--     ├╌╌╌╌╌╌╌╌╌╌╌╌╌╌╮ │                  │     │     -->
<--     │              │ │                  │     │     -->
<--     │◀╌╌╌╌╌╌╌╌╌╌╌╌╌╯ │                  │     │     -->
<--     │                │                  │     │     -->
<--     │           styled arrow            │     │     -->
<--     ├╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶╶≻│     │     -->
<--     │                │                  │     │     -->
<--     │                │ self connecting  │     │     -->
<--     │                │ with             │     │     -->
<--     │                │ new line         │     │     -->
<--     │                ├╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╮ │     │     -->
<--     │                │                │ │     │     -->
<--     │                │◀╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╯ │     │     -->
<--     │                │                  │     │     -->
<--     │              yes? sd              │     │     -->
<--     ├╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌▶│     │     -->
<--     │                │                  │     │     -->
<--     │                no                 │     │     -->
<--     │≺╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴┤     │     -->
<--     │                │                  │     │     -->
<--     │                │  reverse arrow   │     │     -->
<--     │                │≺╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴╴┤     │     -->
<--     │                │                  │     │     -->
<--     │                │         EEEE     │     │     -->
<--     │                │         E        │     │     -->
<--     │                │         EEEE     │     │     -->
<--     │                │         E        │     │     -->
<--     │                │         EEEE     │     │     -->
<--     │                │◀╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┤     -->
<--     │                │                  │     │     -->
<--     │                │                  │     │     -->
<--     │                │                  │     │     -->
<--     │                │                  │     │     -->
<--     │                │                  │     │     -->
<--     │                │                  │     │     -->
===========================================================

**** with al option ****

$ ~/sequence-diagram-cli > seqdia tests/test.txt al prefix='<--'
=====================================================
//        +------------------------+
//+------+|  * TODO                |   +-----++---+
//| User ||  - clean living room   |   | Dev || S |
//+---+--++------------+-----------+   +--+--++-+-+
//    |                |                  |     |
//    |                |                  |     |
//    | default arrow  |                  |     |
//    +--------------+ |                  |     |
//    |              | |                  |     |
//    |<-------------+ |                  |     |
//    |                |                  |     |
//    |           styled arrow            |     |
//    +..................................>|     |
//    |                |                  |     |
//    |                | self connecting  |     |
//    |                | with             |     |
//    |                | new line         |     |
//    |                +----------------+ |     |
//    |                |                | |     |
//    |                |<---------------+ |     |
//    |                |                  |     |
//    |              yes? sd              |     |
//    +---------------------------------->|     |
//    |                |                  |     |
//    |                no                 |     |
//    |<..................................+     |
//    |                |                  |     |
//    |                |  reverse arrow   |     |
//    |                |<.................+     |
//    |                |                  |     |
//    |                |         EEEE     |     |
//    |                |         E        |     |
//    |                |         EEEE     |     |
//    |                |         E        |     |
//    |                |         EEEE     |     |
//    |                |<-----------------------+
//    |                |                  |     |
//    |                |                  |     |
//    |                |                  |     |
//    |                |                  |     |
//    |                |                  |     |
//    |                |                  |     |
=====================================================

```
## Syntax
### participant
You don't need to define participant, but for alignment, you should.
```scala
participant User // this will be front
; or
participant "with a2a!@#!$" as A // you can use this like 'A -> User: ***'
; or
participant "new
line" as nl // this will be last
```

### arrow
```scala
User->New: message // participant 'New' will defined automatically at here
; or
User-->New: message
; or
User<-A: message
; or
User<--A: message
; or
nl -> nl: " realllly // with multiple lines
looooooong
messageee"
; or
ee <- ee: "
  you can't use like this" // when using new line, you should add 1 character to first line at least. I'll fix this soon.
```

### TODO
 - note
 - more arrow design
