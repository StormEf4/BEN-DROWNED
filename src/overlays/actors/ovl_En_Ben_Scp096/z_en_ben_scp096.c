/*
 * File: z_en_ben_scp096.c
 * Overlay: ovl_En_Ben_Scp096
 * Description: BEN DROWNED, the Expanded Horror: SCP-096, the shy one (BEN_EXPANDED; DECISIONS D18). What it does is
 * ovl_En_Ben_Visitor/z_en_ben_scp.inc, shared with the other two; this overlay holds its own model, so only it is in
 * memory while it's here. Its models: tools/ben/gen_ben_visitors.py.
 */

#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_visitor.h"
#include "ben/ben.h"

#define EN_BEN_SCP BEN_VISITOR_SHY

#include "z_en_ben_scp096_models.inc.c"
#include "overlays/actors/ovl_En_Ben_Visitor/z_en_ben_scp.inc"

ActorProfile En_Ben_Scp096_Profile = {
    /**/ ACTOR_EN_BEN_SCP096,
    /**/ ACTORCAT_PROP,
    /**/ FLAGS,
    /**/ GAMEPLAY_KEEP,
    /**/ sizeof(EnBenVisitor),
    /**/ EnBenScp_Init,
    /**/ EnBenScp_Destroy,
    /**/ EnBenScp_Update,
    /**/ EnBenScp_Draw,
};
