/*
 * File: z_en_ben_smile.c
 * Overlay: ovl_En_Ben_Smile
 * Description: BEN DROWNED, the Expanded Horror: the Smiling Man (BEN_EXPANDED; DECISIONS D17). What he does is
 * ovl_En_Ben_Visitor/z_en_ben_visitor.inc, shared with the other two visitors; this overlay holds his own model, so
 * only it is in memory while he's here. Its models: tools/ben/gen_ben_visitors.py.
 */

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.h"
#include "ben/ben.h"

#define EN_BEN_VISITOR BEN_VISITOR_SMILE

#include "z_en_ben_smile_models.inc.c"
#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.inc"

ActorProfile En_Ben_Smile_Profile = {
    /**/ ACTOR_EN_BEN_SMILE,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBenVisitor),
    /**/ EnBenVisitor_Init,
    /**/ EnBenVisitor_Destroy,
    /**/ EnBenVisitor_Update,
    /**/ EnBenVisitor_Draw,
};
