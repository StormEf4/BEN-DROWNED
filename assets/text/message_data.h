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
