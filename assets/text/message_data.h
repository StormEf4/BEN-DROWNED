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

// BEN_TEXT_COLD: Session 4's end, at Darmani's grave in the Goron Graveyard
DEFINE_MESSAGE(0x4D04, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Nobody could heal him either." BOX_BREAK
"So he stays where he fell," "\n"
"and waits for someone" "\n"
"to come and play." BOX_BREAK
DELAY(0x0028) COLOR_RED "Like me." COLOR_DEFAULT
)
)

// BEN_TEXT_MOON: Session 5's end, on the Moon (STORY_BIBLE.md C18)
DEFINE_MESSAGE(0x4D05, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Do you want to play with us?" BOX_BREAK
"No?" BOX_BREAK
"That's all right." "\n"
"Your friend is waiting" "\n"
"at the stone tower."
)
)

// BEN_TEXT_DREAM: Session 6's end, the dream of being made into a statue (STORY_BIBLE.md C20)
DEFINE_MESSAGE(0x4D06, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Hold still." BOX_BREAK
"It's easier if you" "\n"
"hold still." BOX_BREAK
DELAY(0x0028) COLOR_RED "There." "\n"
"Now you look like me." COLOR_DEFAULT
)
)

// BEN_TEXT_LETS_PLAY: Session 7, on top of the Clock Tower
DEFINE_MESSAGE(0x4D07, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "Let's play." COLOR_DEFAULT
)
)

// BEN_TEXT_COUNTER: the end (STORY_BIBLE.md C21)
DEFINE_MESSAGE(0x4D08, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"The counter resets." BOX_BREAK
DELAY(0x0028) "I'm glad you did that."
)
)

// BEN_TEXT_RUMOR: Session 1, a townsperson at night, turned to face you a moment too long (STORY_BIBLE.md §3)
DEFINE_MESSAGE(0x4D09, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"...Have you heard?" BOX_BREAK
"On the last night, when the song" "\n"
"that hurries time won't hurry" "\n"
"it any more..." BOX_BREAK
"...you play it once more." BOX_BREAK
"They say there's a fourth day." "\n"
"They say a boy found it." BOX_BREAK
DELAY(0x0014) "He never came home."
)
)

// BEN_TEXT_HEALING: the statue, the one time you play it the Song of Healing
DEFINE_MESSAGE(0x4D0A, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "That song doesn't work on me." COLOR_DEFAULT
)
)

// BEN_TEXT_HINT_DOUBLE_TIME: Tatl, Session 1 (how to find the 4th day)
DEFINE_MESSAGE(0x4D10, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Everyone's whispering about a" "\n"
"fourth day. It's stupid. Right?" BOX_BREAK
"They say on the last night you" "\n"
"play the Song of Time with every" "\n"
"note twice..." BOX_BREAK
COLOR_LIGHTBLUE "[C-Right][C-Right][A][A][C-Down][C-Down]" COLOR_DEFAULT BOX_BREAK
"...and when it won't go any" "\n"
"further, you play it again." BOX_BREAK
"Don't. ...Okay?"
)
)

// BEN_TEXT_HINT_WATER: Tatl, the 4th day (the Laundry Pool)
DEFINE_MESSAGE(0x4D11, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Where did everybody go?" BOX_BREAK
"That thing keeps following us." "\n"
"It doesn't walk. It's just..." "\n"
"closer." BOX_BREAK
"Maybe it can't follow us into" "\n"
"the water. The Laundry Pool..."
)
)

// BEN_TEXT_HINT_DONT_LOOK: Tatl, the 4th day, after the Laundry Pool
DEFINE_MESSAGE(0x4D12, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"It's right behind us." BOX_BREAK
"Don't look at it." BOX_BREAK
COLOR_RED "DON'T LOOK AT IT." COLOR_DEFAULT
)
)

// BEN_TEXT_HINT_GO_BACK: Tatl, the 4th day, after Link burned
DEFINE_MESSAGE(0x4D13, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"We have to go back." BOX_BREAK
"Play the Song of Time." "\n"
"Please. Please just make it" "\n"
"stop."
)
)

// BEN_TEXT_HINT_SWAMP: Tatl, Session 3
DEFINE_MESSAGE(0x4D14, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Why are you a Deku again?" BOX_BREAK
"...South. Past the field, to the" "\n"
"swamp. The palace." BOX_BREAK
"I don't know how I know that." "\n"
"The King's waiting for you."
)
)

// BEN_TEXT_HINT_GRAVE: Tatl, Session 4
DEFINE_MESSAGE(0x4D15, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"It's so cold up here." BOX_BREAK
"The Gorons had a hero." "\n"
"Darmani. Nobody could heal him." BOX_BREAK
"They buried him past the frozen" "\n"
"lake. He wants to see you." BOX_BREAK
"...And if we don't go to him," "\n"
"it'll bring us."
)
)

// BEN_TEXT_HINT_SEA: Tatl, Session 5
DEFINE_MESSAGE(0x4D16, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"The sea's so calm today." BOX_BREAK
"Swim out. Where it's deep." "\n"
"Further than that." BOX_BREAK
"It's waiting out there." "\n"
"It's always been waiting there."
)
)

// BEN_TEXT_HINT_STONE: Tatl, Session 6
DEFINE_MESSAGE(0x4D17, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Every time you see it, you" "\n"
"forget which way is which." BOX_BREAK
"Don't run from it." "\n"
"Let it find you." BOX_BREAK
"...It wants you to see."
)
)

// BEN_TEXT_HINT_TOWER: Tatl, Session 7
DEFINE_MESSAGE(0x4D18, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"The tower opens at midnight." BOX_BREAK
"He's up there." BOX_BREAK
COLOR_RED "He wants to play." COLOR_DEFAULT
)
)

// BEN_TEXT_HINT_BOOT: Tatl, on BEN's own file
DEFINE_MESSAGE(0x4D19, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"w h o" BOX_BREAK
"a r e   y o u"
)
)

// BEN_TEXT_SEEK: hide and seek, once a session from Session 4 to 6. It stands right in front of you.
DEFINE_MESSAGE(0x4D20, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "Let's play hide and seek." COLOR_DEFAULT BOX_BREAK
COLOR_RED "I hide. You look." COLOR_DEFAULT
)
)

// BEN_TEXT_TAUGHT: after the statue answers the Song of Healing by playing it back, backwards
DEFINE_MESSAGE(0x4D21, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"...Did you hear that?" BOX_BREAK
"It played your song back to you." "\n"
DELAY(0x0014) "Backwards."
)
)

// BEN_TEXT_DONT_MOVE: the second sigil. A Moon Child, close in front of you.
DEFINE_MESSAGE(0x4D22, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Stand still with us." BOX_BREAK
"If you move, he'll know" "\n"
"where you are."
)
)

// BEN_TEXT_NOT_YET: the Song of Unhealing, with nothing for it this session
DEFINE_MESSAGE(0x4D23, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"...not tonight."
)
)

// BEN_TEXT_SIGIL_1 and on: the Moon Children's phrase, a word at a time, as each sigil is stamped
DEFINE_MESSAGE(0x4D24, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "THE MOON" COLOR_DEFAULT " . . . . . ." BOX_BREAK
"Play it again another night," "\n"
"and don't move."
)
)

DEFINE_MESSAGE(0x4D25, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "THE MOON SEES" COLOR_DEFAULT " . . . . ." BOX_BREAK
"Play it again another night," "\n"
"and follow the wet feet."
)
)

DEFINE_MESSAGE(0x4D26, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
COLOR_RED "THE MOON SEES THE CHILD" COLOR_DEFAULT BOX_BREAK
". . . ." BOX_BREAK
"The rest of us are further in."
)
)

// BEN_TEXT_OPENING: Tatl, the first time a new file steps out into South Clock Town. What there is to do, that there
// might be more to it, and that C-up always knows where to go.
DEFINE_MESSAGE(0x4D30, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"...Hey. " DELAY(0x000A) "HEY." BOX_BREAK
"You've been standing here" "\n"
"staring at that clock for ages." BOX_BREAK
"We got your ocarina back." "\n"
"Three days, and then the moon" "\n"
"falls. That's all there is." BOX_BREAK
DELAY(0x0014) "...Right?" BOX_BREAK
"If you don't know where to go," "\n"
"listen for me, and press " COLOR_YELLOW "[C-Up]" COLOR_DEFAULT "." "\n"
"I'll know." BOX_BREAK
DELAY(0x0014) "I always know." "\n"
"I don't know how."
)
)

// BEN_TEXT_TATL_1 to _4: Tatl, out of nowhere, somewhere the story isn't (z_en_ben_termina.inc)
DEFINE_MESSAGE(0x4D31, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Who were you talking to" "\n"
"just now?" BOX_BREAK
DELAY(0x0014) "...You were. I heard you."
)
)

DEFINE_MESSAGE(0x4D32, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Have we been here before?" BOX_BREAK
"It feels like we've been here" "\n"
"before. A lot of times."
)
)

DEFINE_MESSAGE(0x4D33, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Don't look behind us." BOX_BREAK
DELAY(0x0014) "...Okay. It's gone."
)
)

DEFINE_MESSAGE(0x4D34, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
"Why does everyone keep" "\n"
"calling you that?" BOX_BREAK
"That's not your name." BOX_BREAK
DELAY(0x0014) "...Is it?"
)
)

// BEN_TEXT_SLEEP (Expanded Horror): in the dark, his face right in yours
DEFINE_MESSAGE(0x4D40, 0x00, 0x00,
MSG(
HEADER(0x0000, 0xFE, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF)
DELAY(0x0014) COLOR_RED "Go to sleep." COLOR_DEFAULT
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
