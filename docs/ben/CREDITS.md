# Credits

## The SCP Foundation

Three of the Expanded Horror's visitors (`En_Ben_Scp173`, `En_Ben_Scp096` and `En_Ben_Scp106`, DECISIONS D18) are
based on articles from the SCP Wiki (https://scpwiki.com), which are licensed under [Creative Commons Attribution-ShareAlike 3.0](https://creativecommons.org/licenses/by-sa/3.0/):

| In the game | Article | Author |
|---|---|---|
| The sculpture | [SCP-173](https://scpwiki.com/scp-173) | Moto42 |
| The shy one | [SCP-096](https://scpwiki.com/scp-096) | Dr Dan |
| The old man | [SCP-106](https://scpwiki.com/scp-106) | Dr Gears |

Their behaviour follows the articles (SCP-173 moving only when no one is looking; SCP-096 and the viewing of its face;
SCP-106 passing through solid matter, leaving corrosion, and retreating from sudden light). Their models
(`tools/ben/gen_ben_visitors.py`, written to each actor's `z_en_ben_scp<number>_models.inc.c`) are
original, built from the articles' written descriptions. No image from the wiki was used; in particular the well-known
SCP-173 photograph is not under the CC licence.

As the licence asks, those parts of this project are shared under the same licence, CC BY-SA 3.0: the actors
`src/overlays/actors/ovl_En_Ben_Scp173/`, `ovl_En_Ben_Scp096/` and `ovl_En_Ben_Scp106/`, the code they share
(`ovl_En_Ben_Visitor/z_en_ben_scp.inc`), their models, and their generator's SCP functions.
