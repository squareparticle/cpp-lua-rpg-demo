# RPG2002-Ship Architecture Atlas

The Atlas website lives in this folder, inside the repository's broader
`docs/` collection. No package installation, build step, or Sites publication
is required to edit or preview it.

## Files

- `index.html`: site shell and navigation.
- `app.js`: documentation routes and article content.
- `styles.css`: responsive presentation.
- `assets/diagrams/`: downloadable standalone SVG diagrams. `game-map.svg` and
  `world-geometry.svg` embed cropped game art as data URIs, because an SVG
  shown through `<img>` cannot load outside files.
- `assets/media/`: art copied from the game; `assets/media/shots/` holds
  screenshots and GIFs listed in [`SHOTLIST.md`](SHOTLIST.md). Missing shots
  render as labelled placeholders.
- `AUDIT.md`: the October 2026 code-vs-docs audit that drove the rewrite.
- `.nojekyll`: serves these files directly on GitHub Pages.

Existing audit and provenance documents remain alongside the site.

## Preview locally

From the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File scripts/preview-docs.ps1
```

Open <http://127.0.0.1:8765/>. Refresh after editing; Ctrl+C stops the server.
Use `-Port 8766` if the default port is busy. Python 3 is required.
VS Code Live Server can also serve `docs/site-docs/index.html`.

`styles.css` provides two themes: **Sticker Book** (default) and **Night
Voyage**, switched from the top bar and remembered per browser. Both share the
same layout tokens; theme-only decoration sits in the two blocks at the end of
the file, so new components only need the shared rules plus any decoration.

The `style-test*.html` pages are isolated design experiments kept for
comparison; they do not affect the Atlas.

## Maintain the Atlas

1. Add article routes to `app.js` and navigation entries to `index.html`.
2. Write for a curious reader: lead with what the player sees, then the
   mechanism, using real files, tags and numbers as examples. Diagrams should
   show a concrete case from the ship where possible, and every `diagram()`
   call gets its own caption.
3. Cite the repository paths establishing each behavior. Mark future or
   undecided behavior explicitly.
4. Use relative asset paths so the same files work locally and beneath the
   GitHub Pages repository URL. Navigation uses URL fragments, so no server
   routing or fallback configuration is needed.
5. Batch edits, check JavaScript syntax with `node --check docs/site-docs/app.js`, and
   inspect affected links/SVGs. Preview visually when changing presentation.
6. Commit the documentation with the normal main-repository workflow.

## Publish when ready

After committing and pushing the site to GitHub, open the repository's
**Settings > Pages**. Select **Deploy from a branch**, then **main** and
**/docs**, and save. The Atlas will be served under
`/RPG2002-Ship/site-docs/` because its entry point is in this folder. GitHub
Pages must be available for the repository's visibility and account plan.

Once enabled, future pushes to that branch update the public site; local
edits and local commits alone do not publish anything. The expected project
URL is <https://squareparticle.github.io/RPG2002-Ship/> unless a custom domain
is configured. The Atlas URL will be
<https://squareparticle.github.io/RPG2002-Ship/site-docs/>. GitHub settings
have not been changed as part of this migration.

The former OpenAI Sites copy is a historical snapshot. Its original local
checkout, Git history, and hosting metadata are preserved in the ignored
`.local-backups/docs-site-original/` directory; it is not the working source.
