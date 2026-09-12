# Reference imagery

Received 2026-09-12: 15 user attachments, **13 unique PNGs**. Attachments
14 and 15 are byte-identical to 13. Originals are preserved without resizing;
`manifest.json` records dimensions, SHA-256 hashes and duplicate mappings.
The regional map original is 7161 x 3857; do not use the chat thumbnail for detail.

## How to use the supplied images

| Image | File | Observed content | Intended use / limit |
|---|---|---|---|
| 1 | farm_overview_01.png | Dense isometric farm, road, waterfront, garden | Broad zoning only; very low resolution |
| 2 | style_promo_02.png | Promotional animal group, bright landscape | Rounded silhouettes, clear colours, friendly tone; no measurable layout |
| 3 | regional_map_03.png | Large network of waterways, paths, small settlements and markers | Regional context only; not the footprint of a single farm |
| 4 | farm_core_04.png | House, silo, barn edge, road, field and fenced pens | Primary local adjacency study; cropped view, not a full farm survey |
| 5 | garden_paths_05.png | Winding garden paths, conifers, autumn trees, shrubs and flowers | Path rhythm and layered planting |
| 6 | decorated_farm_06.png | Decorated house frontage, small water features, layered planting | Entrance composition and detail density; preserve walking clearance in 3D |
| 7 | cow_shape_07.png | Cow shaded model and wireframe | Large muzzle, eyes, rounded torso; image is not a mesh asset |
| 8 | animal_lighting_08.png | Cow/pigs, naturalistic fur, fence, grass, shallow focus | Optional material/light reference; avoid treating cinematic detail as runtime requirement |
| 9 | cat_shape_09.png | Cat shaded model and wireframe | Face, body and tail silhouette |
| 10 | lobster_shape_10.png | Lobster shaded model and wireframe | Shape language only; does not add lobster gameplay to scope |
| 11 | decor_props_11.png | Stylized flowers, wooden water container, luggage and picnic objects | Rounded edges, material colour separation, clustered decoration |
| 12 | chicken_shape_12.png | Two chicken views | Tall neck, oversized eyes, simple wings and feet |
| 13-15 | horse_shape_13.png | Same horse image repeated | Horse proportions; checkered background is visible in the supplied image |

## Working interpretation for this project

These are multiple scenes and art styles, not different views of one confirmed
farm. Use image 4 as the initial local arrangement reference: road and field in
front, house near silo/barn, pens behind, water at the left edge in the image.
Image directions describe screen position, not verified world compass bearings.
Use images 5/6 as planting and path references without pretending their layouts
connect to image 4. Keep observation, inference and design choices separate.

Start with the existing 162 cm standing eye height. World dimensions and path
widths must be calibrated in-engine; pixel sizes from isometric screenshots are
not physical measurements. Preserve readable gaps and views across fences.
Initial artistic direction inferred from the supplied set: rounded forms,
bright greens, warm earth, white fence accents and red building accents;
natural lighting may add depth without obscuring simple shapes.

## Current readiness

The **absence-of-references blocker is resolved**. Reference analysis and a
limited core-area blockout can now be grounded in supplied imagery. The full
farm extent, hidden building faces and evening lighting remain uncertain.
First Unreal build and movement verification still precede engine blockout.
No level or production mesh is delivered by this reference intake.

## Further sources

See [RESEARCH.md](RESEARCH.md) for independently found primary-source links,
verification limits and what each may help resolve. User-provided discovery
links are retained in manifest.json; their pages were not accessible to the
web reader during this session. Individual artwork credits are not inferred
from those search/board links.

## Storage

Images use the existing Git LFS attributes. These are visual references, not
imported game assets. Original artwork URLs for individual supplied images
were not provided; preserve visible credits and do not invent attribution.
