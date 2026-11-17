/*
 * File: z_en_ben_visitor.c
 * Overlay: ovl_En_Ben_Visitor
 * Description: BEN DROWNED, the Expanded Horror: the Tall Man (BEN_EXPANDED; DECISIONS D17). What he does is
 * ovl_En_Ben_Visitor/z_en_ben_visitor.inc, shared with the other two visitors; this overlay holds his own model, so
 * only it is in memory while he's here. Its models: tools/ben/gen_ben_visitors.py.
 */

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.h"
#include "ben/ben.h"

#define EN_BEN_VISITOR BEN_VISITOR_TALL

#include "z_en_ben_visitor_models.inc.c"
#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.inc"

ActorProfile En_Ben_Visitor_Profile = {
    /**/ ACTOR_EN_BEN_VISITOR,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBenVisitor),
    /**/ EnBenVisitor_Init,
    /**/ EnBenVisitor_Destroy,
    /**/ EnBenVisitor_Update,
    /**/ EnBenVisitor_Draw,
};
