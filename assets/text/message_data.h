#include "text/message_data.h"

/*
 * BEN DROWNED: custom messages (0x4D00-0x4DFF, below the credits range at 0x4E20; tools/ben/check_text_ids.py confirms vanilla doesn't use them).
 * IDs are named in include/ben/ben.h.
 */

// BEN_TEXT_TERRIBLE_FATE: spoken by the statue (En_Ben) in the Phase 0.3 spike
DEFINE_MESSAGE(0x4D00, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"You've met with a terrible fate," "\n"
"haven't you?" BOX_BREAK
"..." DELAY(0x0028) COLOR_RED NAME COLOR_DEFAULT "."
)
)

// BEN_TEXT_SHOULDNT_HAVE: the statue, in front of you, after you play the Song of Time (demo teaser)
DEFINE_MESSAGE(0x4D01, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "YOU SHOULDN'T HAVE DONE THAT." COLOR_DEFAULT
)
)

// BEN_TEXT_STONE: on BEN's own file, at Stone Tower, before anything else happens (STORY_BIBLE.md C3)
DEFINE_MESSAGE(0x4D02, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"St  o n e" BOX_BREAK
"t  t   t  [Z][Z]  o   w e r" "\n"
"  ...[C-Down]  e e e  [A]" "\n"
"w   h  e  r  e  .   .   ." BOX_BREAK
DELAY(0x0028) COLOR_RED "B E N" COLOR_DEFAULT
)
)

// BEN_TEXT_SALESMAN: Session 3, the Happy Mask Salesman, standing behind Deku Link at dawn (STORY_BIBLE.md C12)
DEFINE_MESSAGE(0x4D03, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Ah... You came back." BOX_BREAK
"You've been playing with him," "\n"
"haven't you?" BOX_BREAK
"He doesn't like it when" "\n"
"you leave, you know." BOX_BREAK
"So he made you something" "\n"
"that can't." DELAY(0x0028)
)
)

/*
 * The following two messages should be kept last and in this order.
 * Message 0xFFFD must be last to not break the message debugger.
 * Message 0xFFFC must be immediately before message 0xFFFD to not break Font_LoadOrderedFont.
 */

DEFINE_MESSAGE(0xFFFC, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
)
)

DEFINE_MESSAGE(0xFFFD, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"end!"
)
)
